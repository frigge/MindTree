#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "data/nodes/data_node.h"
#include "data/cache_main.h"
#include "data/raytracing/ray.h"
#include "../render/renderpass.h"

#include "widgets.h"

using namespace MindTree;

std::vector<Widget3D::Factory_t> Widget3D::_widget_factories;

Widget3D::Widget3D(NodeType type) :
    _node(nullptr),
    _renderer(nullptr),
    _screenOriented(false),
    _screenSize(true),
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
                    if (socket->getNode() == _node)
                        this->update();
                 });
    _callbacks.push_back(cb);
    _callbacks.push_back(cb2);
}

Widget3D::~Widget3D()
{
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

    auto model = _renderer->getGlobalTransformation();
    auto view = cam->getViewMatrix();
    auto projection = cam->getProjection();
    auto viewProj = projection * view;
    auto camPos = cam->getPosition();

    //extract translation from model
    glm::mat4 translate = model * glm::inverse(glm::mat4(glm::mat3(model)));

    auto project = [&] (glm::vec4 pos) -> glm::vec2 {
        glm::vec4 posndc = viewProj * translate * pos;
        posndc /= posndc.w;
        glm::vec2 pospixel = posndc.xy() + glm::vec2(1);
        pospixel /= 2;
        pospixel *= viewportSize;
        return pospixel;
    };

    //compute Screen Orientation
    glm::mat4 camrotation = glm::mat4(glm::mat3(view));
    glm::mat4 invcamrot = glm::inverse(camrotation);

    glm::mat4 finalTransform = translate;

    //compute scale matrix
    if(_screenOriented) {
        finalTransform = translate * invcamrot;
    }

    if (_screenSize) {
        auto scale = glm::mat4(1);

        glm::vec2 x0 = project(invcamrot * glm::vec4(0, 0, 0, 1));
        glm::vec2 x1 = project(invcamrot * glm::vec4(1, 0, 0, 1));

        auto vec = x1 - x0;
        if(vec == glm::vec2(0)) {
            if (hitpoint)
                *hitpoint = glm::vec3(0);
            return false;
        }
        float screenlength = glm::length(vec);

        scale[0].x = 50 / screenlength;
        scale[1].y = 50 / screenlength;
        scale[2].z = 50 / screenlength;
    
        finalTransform = finalTransform * scale;
    }


    Ray r = Ray::primaryRay(viewProj, camPos, pixel, viewportSize);
    bool hit = false;
    for(auto shape : _shapes) {
        switch(shape->type()) {
            case Shape::PLANE:
                {
                    auto plane = std::static_pointer_cast<Plane>(shape);
                    hit = r.intersectPlane(*plane, hitpoint);
                    break;
                }
            case Shape::RECTANGLE:
                {
                    auto rect = std::static_pointer_cast<Rectangle>(shape);
                    auto bl = (finalTransform * glm::vec4(rect->bottomLeft(), 1)).xyz();
                    auto u = (finalTransform * glm::vec4(rect->uvector(), 0)).xyz();
                    auto v = (finalTransform * glm::vec4(rect->vvector(), 0)).xyz();
                    auto transRect = Rectangle(bl, u, v);
                    hit = r.intersectRectangle(transRect, hitpoint);
                    break;
                }
            case Shape::SPHERE:
                {
                    auto sphere = std::static_pointer_cast<Sphere>(shape);
                    hit = r.intersectSphere(*sphere, hitpoint);
                    break;
                }
            case Shape::BOX:
                {
                    auto box = std::static_pointer_cast<Box>(shape);
                    hit = r.intersectBox(*box, hitpoint);
                    break;
                }
            case Shape::NONE:
                break;
        }
        if(hit) break;
    }

    return hit;
}

bool Widget3D::checkMousePressed(const std::shared_ptr<Camera> cam, 
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

bool Widget3D::checkMouseMoved(const std::shared_ptr<Camera> cam, 
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

bool Widget3D::checkMouseReleased(const std::shared_ptr<Camera> cam, 
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

void Widget3D::addShape(std::shared_ptr<Shape> shape)
{
    _shapes.push_back(shape);
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
    if(!_renderer) createRenderer();
    _renderer->setVisible(_visible);
    _renderer->setFixedScreenSize(true);
    _renderer->setBorderColor(_outBorderColor);
    _renderer->setFillColor(_outFillColor);
    return _renderer;
}

GL::ShapeRendererGroup* Widget3D::createRenderer()
{
    return new GL::ShapeRendererGroup();
}

void Widget3D::setNode(DNode *node)
{

    if(node->getType() == _type){
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

    auto transsoc = _node->getInSockets()[1];
    auto rotsoc = _node->getInSockets()[2];
    auto scalesoc = _node->getInSockets()[3];

    glm::vec3 translate, scale, rot;
    if(!transsoc->getCntdSocket()) 
        translate = transsoc->getProperty().getData<glm::vec3>();

    if(!rotsoc->getCntdSocket()) 
        rot = rotsoc->getProperty().getData<glm::vec3>();

    if(!scalesoc->getCntdSocket()) 
        scale = scalesoc->getProperty().getData<glm::vec3>();

    auto rotx = glm::rotate(glm::mat4(), rot.x, glm::vec3(1, 0, 0));
    auto roty = glm::rotate(glm::mat4(), rot.y, glm::vec3(0, 1, 0));
    auto rotz = glm::rotate(glm::mat4(), rot.z, glm::vec3(0, 0, 1));

    auto rotmat = rotz * roty * rotx;

    auto scalemat = glm::scale(glm::mat4(), scale);
    auto trans = glm::translate(glm::mat4(), translate);
    auto newtrans = trans * scalemat * rotmat;
    _renderer->setTransformation(newtrans);
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

bool Widget3DManager::mousePressEvent(CameraPtr cam, glm::ivec2 pos, glm::ivec2 viewportSize)
{
    float lastDepth = -1;
    for(auto widget : _widgets) {
        float depth = 0;
        if (widget->checkMousePressed(cam, pos, viewportSize, &depth)) {
            if(lastDepth < 0 || lastDepth > depth) {
                lastDepth = depth;
            }
        }
    }
    return lastDepth > 0;
}

bool Widget3DManager::mouseMoveEvent(CameraPtr cam, glm::ivec2 pos, glm::ivec2 viewportSize)
{
    Widget3DPtr clicked_widget;
    float lastDepth = -1;
    for(auto widget : _widgets) {
        float depth = 0;
        if (widget->checkMouseMoved(cam, pos, viewportSize, &depth)) {
            if(lastDepth < 0 || lastDepth > depth) {
                lastDepth = depth;
                clicked_widget = widget;
            }
        }
    }

    if (clicked_widget) {
        for (auto widget : _widgets) {
            if (widget != clicked_widget) {
                widget->forceHoverLeft();
            }
        }
        return true;
    }
    return false;
}

void Widget3DManager::mouseReleaseEvent()
{
    for(auto widget : _widgets) {
        widget->forceMouseReleased();
    }
}

TranslateWidget::TranslateWidget(Axis axis)
    : Widget3D("TRANSFORM"),
    _axis(axis)
{
    switch(axis) {
        case X:
        case YZ:
            {
                _hoverBorderColor = glm::vec4(1, 0, 0, 1);
                _outBorderColor = glm::vec4(.7, .2, .2, 1);
                _hoverFillColor = glm::vec4(1, 0, 0, .8);
                _outFillColor = glm::vec4(.8, .3, .3, .6);
                break;
            }
        case Y:
        case XZ:
            {
                _hoverBorderColor = glm::vec4(1, 0, 0, 1).grba();
                _outBorderColor = glm::vec4(.7, .2, .2, 1).grba();
                _hoverFillColor = glm::vec4(1, 0, 0, .8).grba();
                _outFillColor = glm::vec4(.8, .3, .3, .6).grba();
                break;
            }
        case Z:
        case XY:
            {
                _hoverBorderColor = glm::vec4(1, 0, 0, 1).gbra();
                _outBorderColor = glm::vec4(.7, .2, .2, 1).gbra();
                _hoverFillColor = glm::vec4(1, 0, 0, .8).gbra();
                _outFillColor = glm::vec4(.8, .3, .3, .6).gbra();
                break;
            }
    }
}

TranslateWidget::~TranslateWidget()
{
}

void TranslateWidget::mouseDraged(glm::vec3 point)
{
    if (!_node) return;
    auto transsocket = _node->getInSockets()[1];
    auto startVal = transsocket->getProperty().getData<glm::vec3>();

    glm::vec3 distance;
    switch(_axis) {
        case X:
            distance.x = point.x - startPoint.x;
            break;
        case Y:
            distance.y = point.y - startPoint.y;
            break;
        case Z:
            distance.z = point.z - startPoint.z;
            break;
        case XY:
        case XZ:
        case YZ:
            distance = point - startPoint;
            break;
    }

    transsocket->setProperty(startVal + distance);
    startPoint = point;
}

