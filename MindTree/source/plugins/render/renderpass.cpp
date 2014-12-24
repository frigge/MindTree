#include "GL/glew.h"
#include "glwrapper.h"
#include "render.h"
#include "polygon_renderer.h"
#include "light_renderer.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "rendermanager.h"
#include "data/debuglog.h"
#include "renderpass.h"

using namespace MindTree;
using namespace MindTree::GL;

RenderPass::RenderPass()
    : _initialized(false), _depthOutput(NONE)
{
}

RenderPass::~RenderPass()
{
    dbout("");
}

void RenderPass::setCamera(CameraPtr camera)
{
    std::lock_guard<std::mutex> lock(_cameraLock);

    _camera = camera;
}

void RenderPass::init()
{
    if(_initialized)
        return;

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
    std::lock_guard<std::mutex> lock(_shapesLock);

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
    assert(group);
    std::lock_guard<std::mutex> lock(_geometryLock);
    //clear all the nodes but leave the shaders there!!
    for (auto node : _geometryShaderNodes)
        node->clear();

    addRenderersFromGroup(group->getMembers());

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

void RenderPass::addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group)
{
    for(const auto &transformable : group) {
        addRendererFromTransformable(transformable);
    }
}

void RenderPass::addRendererFromTransformable(AbstractTransformablePtr transformable)
{
    assert(transformable);
    switch(transformable->getType()) {
        case AbstractTransformable::GEO:
            addRendererFromObject(std::dynamic_pointer_cast<GeoObject>(transformable));
            break;
        case AbstractTransformable::LIGHT:
            addRendererFromLight(std::dynamic_pointer_cast<Light>(transformable));
            break;
        case AbstractTransformable::CAMERA:
            addRendererFromCamera(std::dynamic_pointer_cast<Camera>(transformable));
            break;
        case AbstractTransformable::EMPTY:
            addRendererFromEmpty(std::dynamic_pointer_cast<Empty>(transformable));
            break;
    }
    addRenderersFromGroup(transformable->getChildren());
}

void RenderPass::addRendererFromObject(GeoObjectPtr obj)
{
        auto data = obj->getData();
        switch(data->getType()){
            case ObjectData::MESH:
                addGeometryRenderer(new PolygonRenderer(obj));
                addGeometryRenderer(new EdgeRenderer(obj));
                addGeometryRenderer(new PointRenderer(obj));
                break;
        }
}

void RenderPass::addRendererFromLight(LightPtr obj)
{
    assert(obj);
    switch(obj->getLightType()) {
        case Light::POINT:
            addGeometryRenderer(new PointLightRenderer(std::dynamic_pointer_cast<PointLight>(obj)));
            break;
        case Light::SPOT:
            addGeometryRenderer(new SpotLightRenderer(std::dynamic_pointer_cast<SpotLight>(obj)));
            break;
        case Light::DISTANT:
            addGeometryRenderer(new DistantLightRenderer(std::dynamic_pointer_cast<DistantLight>(obj)));
            break;
    }

}

void RenderPass::addRendererFromCamera(CameraPtr obj)
{
    addGeometryRenderer(new CameraRenderer(obj));
}

void RenderPass::addRendererFromEmpty(EmptyPtr obj)
{
    addGeometryRenderer(new EmptyRenderer(obj));
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
    if(!_initialized) init();

    {
        if(_depthOutput == NONE)
           glDisable(GL_DEPTH_TEST);
        MTGLERROR;

        GLObjectBinder<std::shared_ptr<FBO>> fbobinder(_target);

        {
            std::lock_guard<std::mutex> lock(_bgColorLock);
            glClearColor(_bgColor.r, _bgColor.g, _bgColor.b, _bgColor.a);
        }

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

        int width = _camera->getWidth();
        int height = _camera->getHeight();

        if(!width || !height) {
            std::cout << "No viewport geometry" << std::endl;
            return;
        }

        glViewport(0, 0, (GLint)width, (GLint)height);
        MTGLERROR;

        {
            std::lock_guard<std::mutex> lock(_geometryLock);

            //render nodes that do not have a corresponding objectdata element (grid, 3d widgets, etc.)
            for(auto node : _shadernodes) {
                dbout("");
                GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(node->program());
                UniformStateManager uniformStates(node->program());
                uniformStates.setFromPropertyMap(getProperties());
                uniformStates.setFromPropertyMap(config.getProperties());

                node->render(_camera, glm::ivec2(width, height), config);
                uniformStates.reset();
            }

            for(auto node : _geometryShaderNodes) {
                dbout("");
                GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(node->program());
                UniformStateManager uniformStates(node->program());
                uniformStates.setFromPropertyMap(getProperties());
                node->render(_camera, glm::ivec2(width, height), config);
                uniformStates.reset();
            }
        }
        if(_depthOutput == NONE)
           glEnable(GL_DEPTH_TEST);
        MTGLERROR;
        processPixelRequests();
    }
}
