#include "GL/glew.h"
#include "glwrapper.h"
#include "render.h"
#include "polygon_renderer.h"
#include "renderpass.h"

using namespace MindTree;
using namespace MindTree::GL;

RenderPass::RenderPass()
    : _viewportChanged(true), _initialized(false), _depthOutput(NONE)
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::init()
{
    _initialized = true;
    //make sure shaderprograms are clean
    for(auto shadernode : _shadernodes) {
        shadernode->init();
    }

    for(auto shadernode : _geometryShaderNodes) {
        shadernode->init();
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
            texture->setWidth(_width); 
            texture->setHeight(_height);
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
            renderbuffer->setWidth(_width);
            renderbuffer->setHeight(_height);
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
                    _depthTexture->setWidth(_width);
                    _depthTexture->setHeight(_height);
                    {
                        GLObjectBinder<std::shared_ptr<Texture2D>> binder(_depthTexture);
                        _target->attachDepthTexture(_depthTexture);
                    }
                    break;
                }

            case RENDERBUFFER:
                {
                    _depthRenderbuffer->setWidth(_width);
                    _depthRenderbuffer->setHeight(_height);
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

void RenderPass::addRenderer(Renderer *renderer)
{
    for(auto shaderNode : _shadernodes) {
        if(shaderNode->program() == renderer->getProgram()) {
            shaderNode->addRenderer(renderer);
            return;
        }
    }

    auto newnode = std::make_shared<ShaderRenderNode>(renderer->getProgram());
    newnode->addRenderer(renderer);
    _shadernodes.push_back(newnode);
}

void RenderPass::addGeometryRenderer(Renderer *renderer)
{
    for(auto shaderNode : _geometryShaderNodes) {
        if(shaderNode->program() == renderer->getProgram()) {
            shaderNode->addRenderer(renderer);
            return;
        }
    }

    auto newnode = std::make_shared<ShaderRenderNode>(renderer->getProgram());
    newnode->addRenderer(renderer);
    _geometryShaderNodes.push_back(newnode);
}

std::vector<std::shared_ptr<ShaderRenderNode>> RenderPass::getShaderNodes()
{
    return _shadernodes;
}

void RenderPass::setRenderersFromGroup(std::shared_ptr<Group> group)
{
    //empty all the nodes but leave the shaders there!!
    for (auto node : _geometryShaderNodes)
        node->clear();

    //add the new objects
    for(auto obj : group->getGeometry()){
        addRendererFromObject(obj);
    }

    //find out which shaders are still unused
    std::vector<std::shared_ptr<ShaderRenderNode>> obsolete;
    for (auto node : _geometryShaderNodes) {
        if(node->renders().size() == 0)
            obsolete.push_back(node);
    }

    //kill all the unused shaders!!
    for (auto node : obsolete)
        _geometryShaderNodes.erase(std::find(begin(_shadernodes), end(_shadernodes), node));
}

void RenderPass::addRendererFromObject(std::shared_ptr<GeoObject> obj)    
{
        auto data = obj->getData();
        switch(data->getType()){
            case ObjectData::MESH:
                addGeometryRenderer(new PolygonRenderer(obj));
                addGeometryRenderer(new EdgeRenderer(obj));
                addGeometryRenderer(new PointRenderer(obj));

                for(auto child : obj->getChildren()){
                    if(child->getType() == AbstractTransformable::GEO)
                        addRendererFromObject(std::static_pointer_cast<GeoObject>(child));
                }
                break;
        }
}

void RenderPass::setSize(int width, int height)
{
    std::lock_guard<std::mutex> lock(_sizeLock);
    if(_width != width || _height != height) {
        _viewportChanged = true;
        _initialized = false;
    }

    _width = width;
    _height = height;
}

glm::ivec2 RenderPass::getResolution()
{
    return glm::ivec2(_width, _height);
}

void RenderPass::setCamera(CameraPtr camera)
{
    _camera = camera;
}

CameraPtr RenderPass::getCamera()
{
    return _camera;
}

void RenderPass::render(const RenderConfig &config)
{
    if(!_initialized) init();

    {
        if(_depthOutput == NONE)
           glDisable(GL_DEPTH_TEST);
        MTGLERROR;

        GLObjectBinder<std::shared_ptr<FBO>> fbobinder(_target);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
        MTGLERROR;
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

        {
            std::lock_guard<std::mutex> lock(_sizeLock);
            if(!_width || !_height) {
                std::cout << "No viewport geometry" << std::endl;
                return;
            }
        }

        if(_viewportChanged) {
            glViewport(0, 0, (GLint)_width, (GLint)_height);
            _viewportChanged = false;
        }

        {
            std::lock_guard<std::mutex> lock(_geometryLock);

            //render nodes that to not have a corresponding objectdata element (grid, 3d widgets, etc.)
            for(auto node : _shadernodes)
                node->render(_camera, glm::ivec2(_width, _height), config);

            for(auto node : _geometryShaderNodes)
                node->render(_camera, glm::ivec2(_width, _height), config);
        }
        if(_depthOutput == NONE)
           glEnable(GL_DEPTH_TEST);
        MTGLERROR;
        processPixelRequests();
    }
}

