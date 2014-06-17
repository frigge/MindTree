#define GLM_SWIZZLE

#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"

#include "rendermanager.h"
#include "polygon_renderer.h"
#include "render.h"

using namespace MindTree::GL;

Renderer::Renderer()
    : _initialized(false), _visible(true)
{
}

Renderer::~Renderer()
{
    auto manager = Context::getSharedContext()->getManager();
    if(_vao && _vao.use_count() == 1) manager->scheduleCleanUp(std::move(_vao));
}

void Renderer::setVisible(bool visible)
{
    _visible = visible;
}

void Renderer::_init()    
{
    initVAO();

    {
        GLObjectBinder<std::shared_ptr<VAO>> binder(_vao);

        _initialized = true;
        init();
    }
}

void Renderer::setTransformation(glm::mat4 trans)
{
    _transformation = trans;
}

glm::mat4 Renderer::getTransformation()
{
    return _transformation;
}

void Renderer::render(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    if(!_visible) return;
    if(!_initialized) _init();

    {
        GLObjectBinder<std::shared_ptr<VAO>> vaoBinder(_vao);

        if(camera) {
            glm::mat4 modelView = camera->getViewMatrix() * _transformation;
            glm::mat4 projection = camera->getProjection();
            program->setUniform("modelView", modelView);
            program->setUniform("projection", projection);
        }

        draw(camera, config, program);
    }
}

RenderPass::RenderPass()
    : _viewportChanged(true), _initialized(false)
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::init()
{
    _initialized = true;
    //make sure shaderprograms are clean
    for(auto shadernode : _shadernodes)
        shadernode->program()->resetTextureSlots();

    //if there are no outputs, were rendering to the default framebuffer
    //so no need to setup anything
    if(_outputs.size() == 0)
        return;

    _target = std::make_shared<FBO>();

    {
        GLObjectBinder<std::shared_ptr<FBO>> binder(_target);
        uint i = 0;
        _outputTextures.clear();
        for (auto out : _outputs) {
            auto texture = std::make_shared<Texture2D>(Texture::RGBA, _width, _height);
            _outputTextures.push_back(texture);
            {
                GLObjectBinder<std::shared_ptr<Texture2D>> binder(texture);
                texture->create();
                _target->attachColorTexture(out, texture);

                //init shader programs
                for(auto shadernode : _shadernodes) {
                    shadernode->program()->bindFragmentLocation(i, out);
                }
            }
            ++i;
        }
        auto depth = std::make_shared<Texture2D>(Texture::DEPTH, _width, _height);
        _depthTexture = depth;
        {
            GLObjectBinder<std::shared_ptr<Texture2D>> binder(depth);
            depth->create();
            _target->attachDepthTexture(depth);
        }
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

void RenderPass::addOutput(std::string out)
{
    _outputs.push_back(out);
}

std::vector<std::string> RenderPass::getOutputs()
{
    return _outputs;
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
                addGeometryRenderer(new PointRenderer(obj));
                addGeometryRenderer(new EdgeRenderer(obj));
                addGeometryRenderer(new PolygonRenderer(obj));

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
        GLObjectBinder<std::shared_ptr<FBO>> fbobinder(_target);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
        MTGLERROR;
        getGLFramebufferError(__PRETTY_FUNCTION__);
        std::vector<GLenum> buffers;
        for(size_t i = 0; i < _outputs.size(); i++) {
            buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        if(_outputs.size() > 0) {
            glDrawBuffers(buffers.size(), &buffers[0]);
            MTGLERROR;
        }

        if(_shadernodes.size() == 0) {
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
            for(auto node : _shadernodes)
                node->render(_camera, config);

            for(auto node : _geometryShaderNodes)
                node->render(_camera, config);
        }
    }
}

ShaderRenderNode::ShaderRenderNode(std::shared_ptr<ShaderProgram> program)
    : _program(program)
{
}

ShaderRenderNode::~ShaderRenderNode()
{
    if(_program) Context::getSharedContext()->getManager()
        ->scheduleCleanUp(_program);
}

void ShaderRenderNode::addRenderer(Renderer *renderer)
{
    _renders.push_back(std::shared_ptr<Renderer>(renderer));
}

void ShaderRenderNode::render(CameraPtr camera, const RenderConfig &config)
{
    if(!_program) return;

    {
        GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(_program);
        for(const auto &renderer : _renders) {
            renderer->render(camera, config, _program);
        }
    }
}

std::shared_ptr<ShaderProgram> ShaderRenderNode::program()
{
    return _program;
}

void ShaderRenderNode::clear()
{
    _renders.clear();
}

std::vector<std::shared_ptr<Renderer>> ShaderRenderNode::renders()
{
    return _renders;
}
