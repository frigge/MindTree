#define GLM_FORCE_SWIZZLE

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glwrapper.h"
#include "data/debuglog.h"

#include "rendertree.h"
#include "render.h"

using namespace MindTree::GL;

Renderer::Renderer()
    : _initialized(false),
      _visible(true),
      _parent(nullptr),
      _resourceManager(nullptr)
{
}

Renderer::~Renderer()
{
}

void Renderer::setVisible(bool visible)
{
    _visible = visible;
}

void Renderer::setResourceManager(ResourceManager *manager)
{
    assert(manager != nullptr);
    _resourceManager = manager;

    for(auto &child : _children)
        child->setResourceManager(manager);
}

ResourceManager* Renderer::getResourceManager()
{
    return _resourceManager;
}

void Renderer::_init(ShaderProgram* program)
{
    for (auto &child : _children)
        child->_init(program);

    _vao = make_resource<VAO>(_resourceManager);

    {
        GLObjectBinder<VAO*> binder(_vao.get());

        _initialized = true;
        init(program);
    }
}

void Renderer::setTransformation(glm::mat4 trans)
{
    std::unique_lock<std::shared_timed_mutex> lock(_transformationLock);
    _transformation = trans;
}

glm::mat4 Renderer::getTransformation()
{
    std::shared_lock<std::shared_timed_mutex> lock(_transformationLock);
    return _transformation;
}

glm::mat4 Renderer::getGlobalTransformation()
{
    std::shared_lock<std::shared_timed_mutex> lock(_transformationLock);
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

    if(_resourceManager) child->setResourceManager(_resourceManager);
    _children.push_back(std::unique_ptr<Renderer>(child));
}

void Renderer::addChild(std::unique_ptr<Renderer> &&child)
{
    child->setParent(this);

    for(const auto &ch : _children)
        if(ch == child)
            return;

    if(_resourceManager) child->setResourceManager(_resourceManager);
    _children.push_back(std::move(child));
}

const Renderer* Renderer::getParent() const
{
    return _parent;
}

Renderer* Renderer::getParent()
{
    return _parent;
}

void Renderer::render(const CameraPtr camera, const RenderConfig &config, ShaderProgram* program)
{
    if(!_visible) return;
    assert(_initialized);

    {
        GLObjectBinder<VAO*> vaoBinder(_vao.get());
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
