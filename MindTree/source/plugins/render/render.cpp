#define GLM_SWIZZLE

#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"

#include "rendermanager.h"
#include "render.h"

using namespace MindTree::GL;

Renderer::Renderer()
    : _initialized(false), _visible(true), _parent(nullptr)
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

glm::mat4 Renderer::getGlobalTransformation()
{
    if(_parent) {
        return _parent->getGlobalTransformation() * _transformation;
    }
    else {
        return _transformation;
    }
}

void Renderer::setParent(Renderer *parent)
{
    if(_parent == parent) return;
    _parent = parent;
    parent->addChild(this);
}

void Renderer::addChild(Renderer *child)
{
    child->setParent(this);
    _children.push_back(std::unique_ptr<Renderer>(child));
}

Renderer* Renderer::getParent()
{
    return _parent;
}

void Renderer::render(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    if(!_visible) return;
    if(!_initialized) _init();

    {
        GLObjectBinder<std::shared_ptr<VAO>> vaoBinder(_vao);

        if(camera) {
            auto model = getGlobalTransformation();
            auto view = camera->getViewMatrix();
            auto projection = camera->getProjection();
            program->setUniform("model", model);
            program->setUniform("view", view);
            program->setUniform("modelView", view * model);
            program->setUniform("projection", projection);
            program->setUniform("mvp", projection * view * model);
        }

        draw(camera, config, program);
    }
    for(const auto &child : _children) {
        child->render(camera, config, program);
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

void ShaderRenderNode::init()
{
    for (auto render : _renders)
        render->_init();
    _program->init();
}

void ShaderRenderNode::addRenderer(Renderer *renderer)
{
    _renders.push_back(std::shared_ptr<Renderer>(renderer));
}

void ShaderRenderNode::render(CameraPtr camera, glm::ivec2 resolution, const RenderConfig &config)
{
    if(!_program) return;

    {
        GLObjectBinder<std::shared_ptr<ShaderProgram>> binder(_program);
        _program->setUniform("resolution", resolution);
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

