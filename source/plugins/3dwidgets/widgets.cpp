#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "data/nodes/data_node.h"
#include "data/cache_main.h"
#include "data/raytracing/ray.h"
#include "../render/renderpass.h"
#include "../render/rendertree.h"

#include "widgets.h"

using namespace MindTree;

std::vector<Widget3D::Factory_t> Widget3D::_widget_factories;

Widget3D::Widget3D(SocketType type) :
    _node(nullptr),
    _renderer(nullptr),
    _type(type),
    _visible(false),
    _hover(false),
    _pressed(false),
    _size(.1)
{
    auto cb = Signal
        ::getHandler<DNode*>()
            .connect("selectionChanged", [this](DNode *node){
                         this->setNode(node);
                     });

    auto cb2 = Signal::getHandler<DinSocket*>()
        .connect("socketChanged", [this](DinSocket *socket){
                    this->update();
                 });
    _callbacks.push_back(cb);
    _callbacks.push_back(cb2);
}

void Widget3D::update()
{
    updateTransformation();
}

bool Widget3D::intersectShapes(const std::shared_ptr<Camera> &cam,
                               glm::ivec2 pixel,
                               glm::ivec2 viewportSize,
                               glm::vec3 *hitpoint)
{
    if(!_visible) return false;
    return shape_.intersect(cam, pixel, viewportSize, hitpoint);
}

bool Widget3D::checkMousePressed(const std::shared_ptr<Camera> &cam,
                                 glm::ivec2 pixel,
                                 glm::ivec2 viewportSize,
                                 float *depth)
{
    glm::vec3 hitpoint;
    bool hit = intersectShapes(cam, pixel, viewportSize, &hitpoint);

    if(hit) {
        _pressed = true;
        startPoint = hitpoint;
        mousePressed(hitpoint);
    }
    if (depth) *depth = glm::length(hitpoint - cam->getPosition());
    return hit;
}

bool Widget3D::checkMouseMoved(const std::shared_ptr<Camera> &cam,
                               glm::ivec2 pixel,
                               glm::ivec2 viewportSize,
                               float *depth)
{
    glm::vec3 hitpoint;
    bool hit = intersectShapes(cam, pixel, viewportSize, &hitpoint);

    if(hit) {
        if(!_hover) {

            if(_renderer) {
                _renderer->setBorderColor(_hoverBorderColor);
                _renderer->setFillColor(_hoverFillColor);
            }
            hoverEntered(hitpoint);
        }
        _hover = true;
        mouseMoved(hitpoint);
    }
    else {
        if(_hover) {
            if(_renderer) {
                _renderer->setBorderColor(_outBorderColor);
                _renderer->setFillColor(_outFillColor);
            }
            hoverLeft();
        }
        _hover = false;
    }
    if(_pressed) mouseDraged(hitpoint);

    if (depth) *depth = glm::length(hitpoint - cam->getPosition());
    return hit;
}

bool Widget3D::checkMouseReleased(const std::shared_ptr<Camera> &cam,
                                  glm::ivec2 pixel,
                                  glm::ivec2 viewportSize)
{
    glm::vec3 hitpoint;
    bool hit = intersectShapes(cam, pixel, viewportSize, &hitpoint);

    if(hit) {
        _pressed = false;
        mouseReleased(hitpoint);
    }
    return hit;
}

void Widget3D::mousePressed(glm::vec3 point)
{
}

void Widget3D::mouseMoved(glm::vec3 point)
{
}

void Widget3D::mouseReleased(glm::vec3 point)
{
}

void Widget3D::mouseDraged(glm::vec3 point)
{

}

void Widget3D::hoverEntered(glm::vec3 point)
{
}

void Widget3D::hoverLeft()
{
}

void Widget3D::toggleVisible()
{
    _visible = !_visible;
    _renderer->setVisible(_visible);
}

void Widget3D::setVisible(bool visible)
{
    _visible = visible;
    if(_renderer) _renderer->setVisible(_visible);
}

GL::ShapeRendererGroup* Widget3D::renderer()
{
    if(!_renderer)
        _renderer = shape_.createRenderer();

    if(_renderer) {
        _renderer->setVisible(_visible);
        _renderer->setFixedScreenSize(true);
        _renderer->setBorderColor(_outBorderColor);
        _renderer->setFillColor(_outFillColor);
    }
    return _renderer;
}

void Widget3D::setNode(DNode *node)
{
    auto ins = node->getInSockets();
    if(std::any_of(begin(ins),
                   end(ins),
                   [this](const DinSocket *socket) {
                       return socket->getType() == this->_type;
                   })){
        _node = node;
        setVisible(true);

        if(!_renderer) return;
        updateTransformation();
        return;
    }
    setVisible(false);
}

void Widget3D::updateTransformation()
{
    if(!_node) return;

    auto obj = DataCache::getCachedData(_node);
    if(obj) {
        auto trans = obj.getData<GeoObjectPtr>()->getTransformation();
        _renderer->setTransformation(trans);
    }
}

void Widget3D::forceHoverLeft()
{
    _hover = false;
    hoverLeft();
}

void Widget3D::forceMouseReleased()
{
    _pressed = false;
}

void Widget3D::registerWidget(Factory_t factory)
{
    _widget_factories.push_back(factory);
}

Widget3DManager::Widget3DManager()
{
    for(const auto &factory : Widget3D::_widget_factories)
        _widgets.push_back(factory());
}

void Widget3DManager::insertWidgetsIntoRenderPass(MindTree::GL::RenderPass *pass)
{
    for(auto &widget : _widgets) {
        pass->addRenderer(widget->renderer());
    }
}

bool Widget3DManager::mousePressEvent(const CameraPtr &cam, glm::ivec2 pos, glm::ivec2 viewportSize)
{
    float lastDepth = -1;
    for(auto &widget : _widgets) {
        float depth = 0;
        if (widget->checkMousePressed(cam, pos, viewportSize, &depth)) {
            if(lastDepth < 0 || lastDepth > depth) {
                lastDepth = depth;
            }
        }
    }
    return lastDepth > 0;
}

bool Widget3DManager::mouseMoveEvent(const CameraPtr &cam, glm::ivec2 pos, glm::ivec2 viewportSize)
{
    Widget3D *clicked_widget;
    float lastDepth = -1;
    for(auto &widget : _widgets) {
        float depth = 0;
        if (widget->checkMouseMoved(cam, pos, viewportSize, &depth)) {
            if(lastDepth < 0 || lastDepth > depth) {
                lastDepth = depth;
                clicked_widget = widget.get();
            }
        }
    }

    if (clicked_widget) {
		DataCache::tryUpdate();
        for (auto &widget : _widgets) {
            if (widget.get() != clicked_widget) {
                widget->forceHoverLeft();
            }
        }
        return true;
    }
    //else {
    //    MindTree::GL::RenderThread::pause();
    //}
    return false;
}

void Widget3DManager::mouseReleaseEvent()
{
    for(auto &widget : _widgets) {
        widget->forceMouseReleased();
    }
}
