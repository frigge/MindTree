#include "GL/glew.h"
#include "glwrapper.h"
#include "render.h"
#include "rendermanager.h"
#include "data/debuglog.h"
#include "shader_render_node.h"
#include "renderpass.h"

using namespace MindTree;
using namespace MindTree::GL;

RenderPass::RenderPass() : 
    _initialized(false), 
    _blendSource(GL_SRC_ALPHA),
    _blendDest(GL_ONE_MINUS_SRC_ALPHA),
    _depthOutput(NONE),
    _blending(true),
    _bgColor(0)
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::setCamera(CameraPtr camera)
{
    std::lock_guard<std::mutex> lock(_cameraLock);

    _camera = camera;
}

void RenderPass::setBlendFunc(GLenum src, GLenum dst)
{
    std::lock_guard<std::mutex> lock(_blendLock);
    _blendSource = src;
    _blendDest = dst;
}

void RenderPass::setEnableBlending(bool value)
{
    _blending = value;
}

void RenderPass::setOverrideProgram(std::shared_ptr<ShaderProgram> program)
{
    std::lock_guard<std::mutex> lock(_overrideProgramLock);
    _overrideProgram = program;
}

void RenderPass::init()
{
    _initialized = true;
    
    //make sure shaderprograms are clean
    {
        std::lock_guard<std::mutex> shapeLock(_shapesLock);
        for(auto shadernode : _shadernodes) {
            shadernode->init();
        }
    }

    {
        std::lock_guard<std::mutex> geoLock(_geometryLock);
        for(auto shadernode : _geometryShaderNodes) {
            shadernode->init();
        }
    }

    //if there are no outputs, were rendering to the default framebuffer
    //so no need to setup anything
    if(_outputTextures.size() == 0 && _outputRenderbuffers.size() == 0)
        return;

    _target = std::make_shared<FBO>();

    {
        GLObjectBinder<std::shared_ptr<FBO>> binder(_target);
        uint i = 0;
        for (auto texture : _outputTextures) {
            texture->setWidth(_camera->getWidth());
            texture->setHeight(_camera->getHeight());
            texture->init();
            {
                GLObjectBinder<std::shared_ptr<Texture2D>> binder(texture);
                _target->attachColorTexture(texture);

                //init shader programs
                for(auto shadernode : _shadernodes) {
                    shadernode->program()->bindFragmentLocation(i, texture->getName());
                }
                for(auto shadernode : _geometryShaderNodes) {
                    shadernode->program()->bindFragmentLocation(i, texture->getName());
                }
            }
            ++i;
        }

        for (auto renderbuffer : _outputRenderbuffers) {
            renderbuffer->setWidth(_camera->getWidth());
            renderbuffer->setHeight(_camera->getHeight());
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
                    _depthTexture->setWidth(_camera->getWidth());
                    _depthTexture->setHeight(_camera->getHeight());
                    _depthTexture->init();
                    {
                        GLObjectBinder<std::shared_ptr<Texture2D>> binder(_depthTexture);
                        _target->attachDepthTexture(_depthTexture);
                    }
                    break;
                }

            case RENDERBUFFER:
                {
                    _depthRenderbuffer->setWidth(_camera->getWidth());
                    _depthRenderbuffer->setHeight(_camera->getHeight());
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

std::vector<glm::vec4> RenderPass::readPixel(std::vector<std::string> names, glm::ivec2 pos)
{
    {
        std::lock_guard<std::mutex> locker(_pixelRequestsLock);
        for(auto name : names)
            _pixelRequests.push(std::make_pair(name, pos));
    }
    bool request = true;
    while(request) {
        {
            std::lock_guard<std::mutex> locker(_pixelRequestsLock);
            request = _pixelRequests.size() > 0;
        }
        //wait for the pixels to be read
    }
    return _requestedPixels;
}

void RenderPass::processPixelRequests()
{
    std::lock_guard<std::mutex> locker(_pixelRequestsLock);
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

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
    std::lock_guard<std::mutex> lock(_shapesLock);
    _shadernodes.push_back(node);
}

void RenderPass::addGeometryShaderNode(std::shared_ptr<ShaderRenderNode> node)
{
    std::lock_guard<std::mutex> lock(_geometryLock);
    _geometryShaderNodes.push_back(node);
}

void RenderPass::addRenderer(Renderer *renderer)
{
    std::lock_guard<std::mutex> lock(_shapesLock);
    std::lock_guard<std::mutex> lock2(_overrideProgramLock);
    if(_overrideProgram) {
        if (_shadernodes.empty()) {
            auto newnode = std::make_shared<ShaderRenderNode>(_overrideProgram);
            _shadernodes.push_back(newnode);
        }
        _shadernodes[0]->addRenderer(renderer);
        return;
    }

    std::shared_ptr<ShaderProgram> prog = renderer->getProgram();
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
    std::lock_guard<std::mutex> lock(_geometryLock);
    std::lock_guard<std::mutex> lock2(_overrideProgramLock);

    if(_overrideProgram) {
        if (_geometryShaderNodes.empty()) {
            auto newnode = std::make_shared<ShaderRenderNode>(_overrideProgram);
            _geometryShaderNodes.push_back(newnode);
        }
        _geometryShaderNodes[0]->addRenderer(renderer);
        return;
    }

    std::shared_ptr<ShaderProgram> prog = renderer->getProgram();
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
    std::lock_guard<std::mutex> lock(_geometryLock);
    //clear all the nodes but leave the shaders there!!
    for (auto node : _geometryShaderNodes)
        node->clear();

}

void RenderPass::clearUnusedShaderNodes()
{
    std::lock_guard<std::mutex> lock(_geometryLock);
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
    return _camera;
}

void RenderPass::setBackgroundColor(glm::vec4 color)
{
    std::lock_guard<std::mutex> lock(_bgColorLock);
    _bgColor = color;
}

void RenderPass::render(const RenderConfig &config)
{
    std::lock_guard<std::mutex> lock(_cameraLock);
    if(!_camera) return;
    int width = _camera->getWidth();
    int height = _camera->getHeight();

    if(!width || !height) {
        std::cout << "No viewport geometry" << std::endl;
        return;
    }

    int currentWidth, currentHeight;
    if(_outputTextures.size() > 0) {
        currentWidth = _outputTextures[0]->width();
        currentHeight = _outputTextures[0]->height();
    }
    if(!_initialized || currentHeight != height || currentWidth != width) init();

    {
        if(_depthOutput != NONE)
           glEnable(GL_DEPTH_TEST);
        else
           glDisable(GL_DEPTH_TEST);
        MTGLERROR;

        {
            std::lock_guard<std::mutex> lock(_blendLock);
            glBlendFunc(_blendSource, _blendDest);
            MTGLERROR;
        }

        GLObjectBinder<std::shared_ptr<FBO>> fbobinder(_target);

        {
            std::lock_guard<std::mutex> lock(_bgColorLock);
            glClearColor(_bgColor.r, _bgColor.g, _bgColor.b, _bgColor.a);
        }

        if(_blending)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);

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
            glDrawBuffer(GL_BACK);
            MTGLERROR;
        }

        if(_shadernodes.size() == 0 && _geometryShaderNodes.size() == 0) {
            std::cout << "RenderPass is empty" << std::endl;
            return;
        }

        glViewport(0, 0, (GLint)width, (GLint)height);
        MTGLERROR;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        MTGLERROR;

        {
            std::lock_guard<std::mutex> lock(_geometryLock);

            //render nodes that do not have a corresponding objectdata element (grid, 3d widgets, etc.)
            for(auto node : _shadernodes) {
                node->init();
                {
                    GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(node->program());
                    for(const auto &p : getProperties())
                        node->program()->setUniformFromProperty(p.first, p.second);
    
                    for(const auto &p : config.getProperties())
                        node->program()->setUniformFromProperty(p.first, p.second);
    
                    node->render(_camera, glm::ivec2(width, height), config);
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
                    node->render(_camera, glm::ivec2(width, height), config);
                }
            }
        }
        MTGLERROR;
        processPixelRequests();
    }
}
