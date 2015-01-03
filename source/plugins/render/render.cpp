#define GLM_SWIZZLE

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
    auto manager = RenderManager::getResourceManager();
    if(_vao && _vao.use_count() == 1) manager->scheduleCleanUp(std::move(_vao));
}

void Renderer::setVisible(bool visible)
{
    _visible = visible;
}

void Renderer::_init(std::shared_ptr<ShaderProgram> program)    
{
    RenderThread::asrt();

    for (auto &child : _children)
        child->_init(program);

    _vao = std::make_shared<VAO>();

    {
        GLObjectBinder<std::shared_ptr<VAO>> binder(_vao);

        _initialized = true;
        init(program);
    }
}

void Renderer::setTransformation(glm::mat4 trans)
{
    std::lock_guard<std::mutex> lock(_transformationLock);
    _transformation = trans;
}

glm::mat4 Renderer::getTransformation()
{
    std::lock_guard<std::mutex> lock(_transformationLock);
    return _transformation;
}

glm::mat4 Renderer::getGlobalTransformation()
{
    std::lock_guard<std::mutex> lock(_transformationLock);
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

    for(const auto &ch : _children)
        if(ch.get() == child)
            return;

    _children.push_back(std::shared_ptr<Renderer>(child));
}

void Renderer::addChild(std::shared_ptr<Renderer>(child))
{
    child->setParent(this);

    for(const auto &ch : _children)
        if(ch == child)
            return;

    _children.push_back(child);
}

const Renderer* Renderer::getParent() const
{
    return _parent;
}

Renderer* Renderer::getParent()
{
    return _parent;
}

void Renderer::render(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    RenderThread::asrt();
    if(!_visible) return;
    assert(_initialized);

    {
        GLObjectBinder<std::shared_ptr<VAO>> vaoBinder(_vao);
        UniformStateManager uniformStates(program);

        if(camera) {
            auto model = getGlobalTransformation();
            auto view = camera->getViewMatrix();
            auto projection = camera->getProjection();
            uniformStates.addState("model", model);
            uniformStates.addState("view", view);
            uniformStates.addState("modelView", view * model);
            uniformStates.addState("projection", projection);
            uniformStates.addState("mvp", projection * view * model);
        }

        draw(camera, config, program);
    }
    for(const auto &child : _children) {
        child->render(camera, config, program);
    }
}
