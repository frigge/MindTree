#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
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
    _size(.1),
	_screenOriented(false),
	_screenSize(false)
{
    auto cb = Signal
        ::getHandler<DNode*>()
            .connect("selectionChanged", [this](DNode *node){
                         this->setNode(node);
                     });

    // auto cb2 = Signal::getHandler<>()
    //     .connect("CACHEUPDATED", [this](){
    //                 this->update();
    //              });
    _callbacks.push_back(cb);
    // _callbacks.push_back(cb2);
}

void Widget3D::update()
{
    updateTransformation();
}

bool Widget3D::intersectShapes(const std::shared_ptr<Camera> &cam,
                               glm::ivec2 pixel,
                               float *depth,
                               glm::vec3 *hitpoint)
{
    if(!_visible) return false;

	auto trans = computeTransformation(cam);
	glm::ivec2 res(cam->getWidth(), cam->getHeight());
	auto ray = Ray::primaryRay(pixel, res, cam->fov());
	ray.start = (trans * glm::vec4(ray.start, 1)).xyz();
	ray.dir = (trans * glm::vec4(ray.dir, 0)).xyz();

	bool hit = shapegroup_.intersect(ray, depth);
	*hitpoint = glm::vec4(ray.start + *depth * ray.dir, 1);
	
	return hit;
}

bool Widget3D::checkMousePressed(const std::shared_ptr<Camera> &cam,
                                 glm::ivec2 pixel,
                                 glm::vec3 *hitpoint,
                                 float *depth)
{
    return intersectShapes(cam, pixel, depth, hitpoint);
}

bool Widget3D::checkMouseMoved(const std::shared_ptr<Camera> &cam,
                               glm::ivec2 pixel,
							   glm::vec3 *hitpoint,
							   float *depth)
{
    return intersectShapes(cam, pixel, depth, hitpoint);
}

void Widget3D::moveMouse(glm::vec3 point)
{
	if (_pressed) mouseDraged(point);
	else mouseMoved(point);
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

bool Widget3D::hoverEntered(glm::vec3 point)
{
	return true;
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
        _renderer = createRenderer();

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
        trans[0] = glm::normalize(trans[0]);
        trans[1] = glm::normalize(trans[1]);
        trans[2] = glm::normalize(trans[2]);
        _renderer->setTransformation(trans);
    }
}

void Widget3D::pressMouse(glm::vec3 point)
{
	mousePressed(point);
	_pressed = true;
	startPoint = point;
}

void Widget3D::enterHover(glm::vec3 hitpoint)
{
	if (_hover) return;

	if(_renderer && hoverEntered(hitpoint)) {
		_renderer->setBorderColor(_hoverBorderColor);
		_renderer->setFillColor(_hoverFillColor);
		_hover = true;
	}
}

void Widget3D::leaveHover()
{
	if (!_hover) return;

	if(_renderer) {
		_renderer->setBorderColor(_outBorderColor);
		_renderer->setFillColor(_outFillColor);
	}
	_hover = false;
    hoverLeft();
}

void Widget3D::forceMouseReleased()
{
    _pressed = false;
    leaveHover();
    update();
}

void Widget3D::registerWidget(Factory_t factory)
{
    _widget_factories.push_back(factory);
}

//camera Space to (scaled and oriented) widget space
glm::mat4 Widget3D::computeTransformation(const std::shared_ptr<Camera> &cam) const
{
    auto model = _renderer->getGlobalTransformation();
    glm::vec4 translate = model[3];

    glm::mat4 finalTransform(1);

    auto view = cam->getViewMatrix();
    if(_screenOriented) {
		finalTransform[3] = -translate;
	}
    else {
        finalTransform = glm::inverse(model);
    }

	auto p = cam->getProjection();
	auto mvp = p * view * model;
    if (_screenSize) {
		float scale = abs(mvp[3].z) * .1;
		finalTransform[0] *= scale;
		finalTransform[1] *= scale;
		finalTransform[2] *= scale;
    }

	auto camModel = cam->getWorldTransformation();
    return finalTransform * camModel;
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

bool Widget3DManager::mousePressEvent(const CameraPtr &cam, glm::ivec2 pos)
{
	float lastDepth = std::numeric_limits<float>::max();
	Widget3D *pressed = nullptr;
	glm::vec3 hitpoint;
	for(auto &widget : _widgets) {
		glm::vec3 p;
		float depth = lastDepth;
		if (widget->checkMousePressed(cam, pos, &p, &depth)) {
            if(lastDepth > depth) {
                lastDepth = depth;
                pressed = widget.get();
                hitpoint = p;
            }
        }
    }
	if (pressed && pressed->_hover) {
		_clicked_widget = pressed;
		pressed->pressMouse(hitpoint);
		return true;
	}
	return false;
}

bool Widget3DManager::mouseMoveEvent(const CameraPtr &cam, glm::ivec2 pos)
{
    glm::vec3 hitpoint;
	Widget3D *hovered = _clicked_widget;
    if (hovered) {
		float depth = std::numeric_limits<float>::max();
		_clicked_widget->checkMouseMoved(cam, pos, &hitpoint, &depth);
	}
	else {
	    float lastDepth = std::numeric_limits<float>::max();
	    for(auto &widget : _widgets) {
		    float depth = lastDepth;
		    glm::vec3 p;
		    if (widget->checkMouseMoved(cam, pos, &p, &depth) && lastDepth > depth) {
				lastDepth = depth;
				hovered = widget.get();
				hitpoint = p;
			}
	    }
    }
    
    if (hovered) {
        for (auto &widget : _widgets) {
            if (widget.get() != hovered) {
                widget->leaveHover();
            }
        }
		hovered->enterHover(hitpoint);
        hovered->moveMouse(hitpoint);
        return true;
    }
    else {
        for (auto &widget : _widgets) {
			widget->leaveHover();
		}
    }
    return false;
}

void Widget3DManager::mouseReleaseEvent()
{
	_clicked_widget = nullptr;
    for(auto &widget : _widgets) {
        widget->forceMouseReleased();
    }
}
