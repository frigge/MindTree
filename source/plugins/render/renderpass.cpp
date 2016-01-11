#include "GL/glew.h"
#include "glwrapper.h"
#include "render.h"
#include "rendertree.h"
#include "data/debuglog.h"
#include "shader_render_node.h"
#include "rendertree.h"
#include "benchmark.h"
#include "renderpass.h"

using namespace MindTree;
using namespace MindTree::GL;

RenderPass::RenderPass() :
    _initialized(false),
    _enabled(true),
    _blendColorSource(GL_SRC_ALPHA),
    _blendAlphaSource(GL_ONE),
    _blendColorDest(GL_ONE_MINUS_SRC_ALPHA),
    _blendAlphaDest(GL_ZERO),
    _blending(true),
    _depthOutput(NONE),
    _bgColor(0),
    _depth(1),
    _tree(nullptr)
{
}

RenderPass::~RenderPass()
{
    for (auto tx : _outputTextures) {
        RenderTree::getResourceManager()->scheduleCleanUp(tx);
    }

    for (auto rb : _outputRenderbuffers) {
        RenderTree::getResourceManager()->scheduleCleanUp(rb);
    }

    RenderTree::getResourceManager()->scheduleCleanUp(_depthTexture);
    RenderTree::getResourceManager()->scheduleCleanUp(_depthRenderbuffer);
    RenderTree::getResourceManager()->scheduleCleanUp(_target);
}

void RenderPass::addPostRenderCallback(std::function<void(RenderPass*)> cb)
{
    _postRenderCallbacks.push_back(cb);
}

void RenderPass::setBenchmark(std::shared_ptr<Benchmark> benchmark)
{
    _benchmark = benchmark;
}

void RenderPass::setEnabled(bool enable)
{
    _enabled = enable;
}

bool RenderPass::isEnabled() const
{
    return _enabled;
}

void RenderPass::setCamera(CameraPtr camera)
{
    std::unique_lock<std::shared_timed_mutex> lock(_cameraLock);

    _camera = camera;
}

void RenderPass::setBlendFunc(GLenum src, GLenum dst)
{
    std::unique_lock<std::shared_timed_mutex> lock(_blendLock);
    _blendColorSource = src;
    _blendAlphaSource = src;
    _blendColorDest = dst;
    _blendAlphaDest = dst;
}

void RenderPass::setBlendFuncSeparate(GLenum srcColor, GLenum srcAlpha, GLenum dstColor, GLenum dstAlpha)
{
    std::unique_lock<std::shared_timed_mutex> lock(_blendLock);
    _blendColorSource = srcColor;
    _blendAlphaSource = srcAlpha;
    _blendColorDest = dstColor;
    _blendAlphaDest = dstAlpha;
}

void RenderPass::setEnableBlending(bool value)
{
    _blending = value;
}

void RenderPass::setOverrideProgram(std::shared_ptr<ShaderProgram> program)
{
    std::unique_lock<std::shared_timed_mutex> lock(_overrideProgramLock);
    _overrideProgram = program;
}

void RenderPass::setTree(RenderTree *tree)
{
    _tree = tree;
}

void RenderPass::init()
{
    _initialized = true;

    {
        std::shared_lock<std::shared_timed_mutex> lock(_cameraLock);
        _currentWidth = getCamera()->getWidth();
        _currentHeight = getCamera()->getHeight();
    }
    //make sure shaderprograms are clean
    {
        std::shared_lock<std::shared_timed_mutex> shapeLock(_shapesLock);
        for(auto shadernode : _shadernodes) {
            shadernode->init();
        }
    }

    {
        std::shared_lock<std::shared_timed_mutex> geoLock(_geometryLock);
        for(auto shadernode : _geometryShaderNodes) {
            shadernode->init();
        }
    }

    //if there are no outputs, were rendering to the default framebuffer
    //so no need to setup anything
    if(_outputTextures.size() == 0 && _outputRenderbuffers.size() == 0
       && _depthOutput == NONE)
        return;

    _target = std::make_shared<FBO>();

    {
        GLObjectBinder<std::shared_ptr<FBO>> binder(_target);
        uint i = 0;
        for (auto texture : _outputTextures) {
            texture->setWidth(_currentWidth);
            texture->setHeight(_currentHeight);
            texture->init();
            {
                GLObjectBinder<std::shared_ptr<Texture2D>> binder(texture);
                _target->attachColorTexture(texture);

                //init shader programs
                for(auto shadernode : _shadernodes) {
                    shadernode->program()->bindFragmentLocation(i, getFragmentName(texture));
                }
                for(auto shadernode : _geometryShaderNodes) {
                    shadernode->program()->bindFragmentLocation(i, getFragmentName(texture));
                }
            }
            ++i;
        }

        for (auto renderbuffer : _outputRenderbuffers) {
            renderbuffer->setWidth(_currentWidth);
            renderbuffer->setHeight(_currentHeight);
            renderbuffer->init();
            {
                GLObjectBinder<std::shared_ptr<Renderbuffer>> binder(renderbuffer);
                _target->attachColorRenderbuffer(renderbuffer);

                //init shader programs
                for(auto shadernode : _shadernodes) {
                    shadernode->program()->bindFragmentLocation(i, renderbuffer->getName());
                }
                for(auto shadernode : _geometryShaderNodes) {
                    shadernode->program()->bindFragmentLocation(i, renderbuffer->getName());
                }
            }
            ++i;
        }

        switch(_depthOutput) {
            case TEXTURE:
                {
                    _depthTexture->setWidth(_currentWidth);
                    _depthTexture->setHeight(_currentHeight);
                    _depthTexture->init();
                    {
                        GLObjectBinder<std::shared_ptr<Texture2D>> binder(_depthTexture);
                        _target->attachDepthTexture(_depthTexture);
                    }
                    break;
                }

            case RENDERBUFFER:
                {
                    _depthRenderbuffer->setWidth(_currentWidth);
                    _depthRenderbuffer->setHeight(_currentHeight);
                    _depthRenderbuffer->init();
                    {
                        GLObjectBinder<std::shared_ptr<Renderbuffer>> binder(_depthRenderbuffer);
                        _target->attachDepthRenderbuffer(_depthRenderbuffer);
                    }
                    break;
                }
            default:
                break;
        }
    }
}

void RenderPass::setDirty()
{
    _initialized = false;
}

void RenderPass::setCustomTextureNameMapping(std::string realname, std::string newname)
{
    {
        std::unique_lock<std::shared_timed_mutex> lock(_textureNameMappingLock);
        _textureNameMappings[realname] = newname;
    }
    _tree->setDirty();
}

void RenderPass::setCustomFragmentNameMapping(std::string realname, std::string newname)
{
    {
        std::unique_lock<std::shared_timed_mutex> lock(_fragmentNameMappingLock);
        _fragmentNameMappings[realname] = newname;
    }
    _tree->setDirty();
}

std::string RenderPass::getTextureName(std::shared_ptr<Texture2D> tex) const
{
    std::shared_lock<std::shared_timed_mutex> lock(_textureNameMappingLock);
    std::string realName = tex->getName();
    if(_textureNameMappings.find(realName) != end(_textureNameMappings)) {
        std::string mappedName = _textureNameMappings.at(realName);
        dbout("redirecting: " << realName << " to: " << mappedName);
        return mappedName;
    }
    else
        return realName;
}

std::string RenderPass::getFragmentName(std::shared_ptr<Texture2D> tex) const
{
    std::shared_lock<std::shared_timed_mutex> lock(_fragmentNameMappingLock);
    std::string realName = tex->getName();
    if(_fragmentNameMappings.find(realName) != end(_fragmentNameMappings))
        return _fragmentNameMappings.at(realName);
    else
        return realName;
}

void RenderPass::clearCustomTextureNameMapping()
{
    {
        std::unique_lock<std::shared_timed_mutex> lock(_textureNameMappingLock);
        _textureNameMappings.clear();
    }
    _tree->setDirty();
}

void RenderPass::clearCustomFragmentNameMapping()
{
    {
        std::unique_lock<std::shared_timed_mutex> lock(_textureNameMappingLock);
        _textureNameMappings.clear();
    }
    _tree->setDirty();
}

void RenderPass::setTextures(std::vector<std::shared_ptr<Texture2D>> textures)
{
    for (auto shadernode : _shadernodes) {
        for(auto texture : textures) {
            std::string name = getTextureName(texture);
            shadernode->program()->setTexture(texture, name);
        }
    }
}

std::vector<glm::vec4> RenderPass::readPixel(std::vector<std::string> names, glm::ivec2 pos)
{
    {
        std::unique_lock<std::shared_timed_mutex> locker(_pixelRequestsLock);
        for(auto name : names)
            _pixelRequests.push(std::make_pair(name, pos));
    }
    bool request = true;
    while(request) {
        {
            std::shared_lock<std::shared_timed_mutex> locker(_pixelRequestsLock);
            request = _pixelRequests.size() > 0;
        }
        //wait for the pixels to be read
    }
    return _requestedPixels;
}

void RenderPass::processPixelRequests()
{
    std::unique_lock<std::shared_timed_mutex> locker(_pixelRequestsLock);
    _requestedPixels.clear();
    while(_pixelRequests.size() > 0) {
        auto request = _pixelRequests.front();
        _pixelRequests.pop();
        auto name = request.first;
        auto pos = request.second;

        //find attachment
        if(_target) {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + _target->getAttachmentPos(name));
            MTGLERROR;
        }
        else {
            glReadBuffer(GL_BACK);
            MTGLERROR;
        }

        //find out whether name is a renderbuffer or a taxture
        GLenum datasize = GL_UNSIGNED_BYTE;
        GLenum format = GL_RGBA;
        for(auto tex : _outputTextures) {
            if (tex->getName() == name){
                switch(tex->getFormat()) {
                    case Texture::RGB:
                        format = GL_RGB;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Texture::RGB8:
                        format = GL_RGB;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Texture::RGBA:
                        format = GL_RGBA;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Texture::RGBA8:
                        format = GL_RGBA;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Texture::RGBA16F:
                        format = GL_RGBA;
                        datasize = GL_FLOAT;
                        break;
                    default:
                        break;
                }
            }
        }

        for(auto renderbuffer : _outputRenderbuffers) {
            if (renderbuffer->getName() == name){
                switch(renderbuffer->getFormat()) {
                    case Renderbuffer::RGB:
                        format = GL_RGB;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Renderbuffer::RGB8:
                        format = GL_RGB;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Renderbuffer::RGBA:
                        format = GL_RGBA;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Renderbuffer::RGBA8:
                        format = GL_RGBA;
                        datasize = GL_UNSIGNED_BYTE;
                        break;
                    case Renderbuffer::RGBA16F:
                        format = GL_RGBA;
                        datasize = GL_FLOAT;
                        break;
                    default:
                        break;
                }
            }
        }
        //and then find its datasize
        void* data = nullptr;

        if(format == GL_RGB)
            data = new GLubyte[3];
        else if(format == GL_RGBA && datasize == GL_UNSIGNED_BYTE)
            data = new GLubyte[4];
        else if(format == GL_RGBA && datasize == GL_FLOAT)
            data = new GLfloat[4];

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        MTGLERROR;
        glReadPixels(pos.x, pos.y, 1, 1, format, datasize, data);
        MTGLERROR;

        glm::vec4 color;
        if(format == GL_RGB) {
                GLubyte* raw_color = reinterpret_cast<GLubyte*>(data);
                color = glm::vec4(raw_color[0] / 255.0,
                                  raw_color[1] / 255.0,
                                  raw_color[2] / 255.0,
                                  1);
                delete [] raw_color;
        }
        else if(format == GL_RGBA && datasize == GL_UNSIGNED_BYTE) {
                GLubyte* raw_color = reinterpret_cast<GLubyte*>(data);
                color = glm::vec4(raw_color[0] / 255.0,
                                  raw_color[1] / 255.0,
                                  raw_color[2] / 255.0,
                                  raw_color[3] / 255.0);
                delete [] raw_color;
        }
        else if(format == GL_RGBA && datasize == GL_FLOAT) {
                GLfloat* raw_color = reinterpret_cast<GLfloat*>(data);
                color = glm::vec4(raw_color[0],
                                  raw_color[1],
                                  raw_color[2],
                                  raw_color[3]);
                delete [] raw_color;
        }

        _requestedPixels.push_back(color);
    }
}

void RenderPass::setTarget(std::shared_ptr<FBO> target)
{
    _target = target;
}

std::shared_ptr<FBO> RenderPass::getTarget()
{
    return _target;
}

void RenderPass::addOutput(std::shared_ptr<Texture2D> tex)
{
    _outputTextures.push_back(tex);
}

void RenderPass::addOutput(std::shared_ptr<Renderbuffer> rb)
{
    _outputRenderbuffers.push_back(rb);
}

void RenderPass::setDepthOutput(std::shared_ptr<Texture2D> output)
{
    _depthTexture = output;
    _depthOutput = TEXTURE;
}

void RenderPass::setDepthOutput(std::shared_ptr<Renderbuffer> output)
{
    _depthRenderbuffer = output;
    _depthOutput = RENDERBUFFER;
}

std::vector<std::shared_ptr<Texture2D>> RenderPass::getOutputTextures()
{
    return _outputTextures;
}

std::shared_ptr<Texture2D> RenderPass::getOutDepthTexture()
{
    return _depthTexture;
}

void RenderPass::addShaderNode(std::shared_ptr<ShaderRenderNode> node)
{
    std::unique_lock<std::shared_timed_mutex> lock(_shapesLock);
    _shadernodes.push_back(node);
}

void RenderPass::addGeometryShaderNode(std::shared_ptr<ShaderRenderNode> node)
{
    std::unique_lock<std::shared_timed_mutex> lock(_geometryLock);
    _geometryShaderNodes.push_back(node);
}

void RenderPass::addRenderer(Renderer *renderer)
{
    std::unique_lock<std::shared_timed_mutex> lock(_shapesLock);
    std::shared_lock<std::shared_timed_mutex> lock2(_overrideProgramLock);
    if(_overrideProgram) {
        if (_shadernodes.empty()) {
            auto newnode = std::make_shared<ShaderRenderNode>(_overrideProgram);
            _shadernodes.push_back(newnode);
        }
        _shadernodes[0]->addRenderer(renderer);
        return;
    }

    std::shared_ptr<ShaderProgram> prog = renderer->getProgram();
    assert(prog);
    for(auto shaderNode : _shadernodes) {
        if(shaderNode->program() == prog) {
            shaderNode->addRenderer(renderer);
            return;
        }
    }

    auto newnode = std::make_shared<ShaderRenderNode>(prog);
    newnode->addRenderer(renderer);
    _shadernodes.push_back(newnode);
}

void RenderPass::addGeometryRenderer(Renderer *renderer)
{
    std::unique_lock<std::shared_timed_mutex> lock(_geometryLock);
    std::shared_lock<std::shared_timed_mutex> lock2(_overrideProgramLock);

    if(_overrideProgram) {
        if (_geometryShaderNodes.empty()) {
            auto newnode = std::make_shared<ShaderRenderNode>(_overrideProgram);
            _geometryShaderNodes.push_back(newnode);
        }
        _geometryShaderNodes[0]->addRenderer(renderer);
        return;
    }

    std::shared_ptr<ShaderProgram> prog = renderer->getProgram();
    assert(prog);
    for(auto shaderNode : _geometryShaderNodes) {
        if(shaderNode->program() == prog) {
            shaderNode->addRenderer(renderer);
            return;
        }
    }

    auto newnode = std::make_shared<ShaderRenderNode>(prog);
    newnode->addRenderer(renderer);
    _geometryShaderNodes.push_back(newnode);
}

void RenderPass::clearRenderers()
{
    std::unique_lock<std::shared_timed_mutex> lock(_geometryLock);
    //clear all the nodes but leave the shaders there!!
    for (auto node : _geometryShaderNodes)
        node->clear();

}

void RenderPass::clearUnusedShaderNodes()
{
    std::unique_lock<std::shared_timed_mutex> lock(_geometryLock);
    //find out which shaders are still unused
    std::vector<std::shared_ptr<ShaderRenderNode>> obsolete;
    for (auto node : _geometryShaderNodes) {
        if(node->renders().empty())
            obsolete.push_back(node);
    }

    //kill all the unused shaders!!
    for (auto node : obsolete)
        _geometryShaderNodes.erase(std::find(begin(_geometryShaderNodes), end(_geometryShaderNodes), node));
}

std::vector<std::shared_ptr<ShaderRenderNode>> RenderPass::getShaderNodes()
{
    return _shadernodes;
}

CameraPtr RenderPass::getCamera()
{
    assert(_camera);
    return _camera;
}

void RenderPass::setBackgroundColor(glm::vec4 color)
{
    std::lock_guard<std::mutex> lock(_bgColorLock);
    _bgColor = color;
}

void RenderPass::setClearDepth(float value)
{
    _depth = value;
}

void RenderPass::render(const RenderConfig &config)
{
    int width{0};
    int height{0};
    {
        std::shared_lock<std::shared_timed_mutex> lock(_cameraLock);
        if(!_camera) return;
        width = _camera->getWidth();
        height = _camera->getHeight();
    }

    if(!width || !height) {
        return;
    }

    BenchmarkHandler bhandler(_benchmark);

    if(!_initialized || _currentHeight != height || _currentWidth != width) init();

    {
        if(_depthOutput != NONE)
           glEnable(GL_DEPTH_TEST);
        else
           glDisable(GL_DEPTH_TEST);
        MTGLERROR;

        {
            glBlendEquation(GL_FUNC_ADD);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            std::shared_lock<std::shared_timed_mutex> lock(_blendLock);
            if(_blendColorSource == _blendAlphaSource
               && _blendColorDest == _blendAlphaDest) {
                glBlendFuncSeparate(GL_ONE, GL_ZERO,  GL_ONE, GL_ZERO);
                glBlendFunc(_blendColorSource, _blendColorDest);
            } else {
                glBlendFunc(GL_ONE, GL_ZERO);
                glBlendFuncSeparate(_blendColorSource, _blendColorDest,  _blendAlphaSource, _blendAlphaDest);
            }
            MTGLERROR;

            if(_blending)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
        }

        GLObjectBinder<std::shared_ptr<FBO>> fbobinder(_target);

        {
            std::lock_guard<std::mutex> lock(_bgColorLock);
            glClearColor(_bgColor.r, _bgColor.g, _bgColor.b, _bgColor.a);
        }
        glClearDepth(_depth);

        getGLFramebufferError(__PRETTY_FUNCTION__);
        MTGLERROR;
        std::vector<GLenum> buffers;

        for(size_t i = 0; i < _outputTextures.size(); i++)
            buffers.push_back(GL_COLOR_ATTACHMENT0 + i);

        for(size_t i = 0; i < _outputRenderbuffers.size(); i++)
            buffers.push_back(GL_COLOR_ATTACHMENT0 + _outputTextures.size() + i);

        if(buffers.size() > 0) {
            glDrawBuffers(buffers.size(), &buffers[0]);
            MTGLERROR;
        }
        else {
            if(!_target) {
                glDrawBuffer(GL_BACK);
            }
            else {
                glDrawBuffer(GL_NONE);
            }
        }

        if(_shadernodes.empty() && _geometryShaderNodes.empty()) {
            std::cout << "RenderPass is empty" << std::endl;
            return;
        }

        glViewport(0, 0, (GLint)width, (GLint)height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(_enabled) {
            {
                std::shared_lock<std::shared_timed_mutex> lock(_geometryLock);
                std::shared_lock<std::shared_timed_mutex> shapeLock(_shapesLock);
                //render nodes that do not have a corresponding objectdata element (grid, 3d widgets, etc.)
                for(auto node : _shadernodes) {
                    node->init();
                    {
                        GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(node->program());
                        for(const auto &p : getProperties())
                            node->program()->setUniformFromProperty(p.first, p.second);

                        for(const auto &p : config.getProperties())
                            node->program()->setUniformFromProperty(p.first, p.second);

                        {
                            std::shared_lock<std::shared_timed_mutex> lock(_cameraLock);
                            node->render(_camera, glm::ivec2(width, height), config);
                        }
                    }
                }

                for(auto node : _geometryShaderNodes) {
                    node->init();
                    {
                        GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(node->program());
                        for(const auto &p : getProperties())
                            node->program()->setUniformFromProperty(p.first, p.second);

                        for(const auto &p : config.getProperties())
                            node->program()->setUniformFromProperty(p.first, p.second);
                        {
                            std::shared_lock<std::shared_timed_mutex> lock(_cameraLock);
                            node->render(_camera, glm::ivec2(width, height), config);
                        }
                    }
                }
            }
            for(auto cb : _postRenderCallbacks)
                cb(this);
        }
        processPixelRequests();
    }
    glFinish();
}
