#define GLM_FORCE_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "data/nodes/data_node.h"
#include "data/cache_main.h"
#include "translate_widgets.h"

using namespace MindTree;

TranslateWidget::TranslateWidget(Axis axis)
    : Widget3D("MAT4"),
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

void TranslateWidget::mouseDraged(glm::vec3 point)
{
    if (!_node || !_pressed) return;

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

    startPoint = point;
    updateValue(distance);
}

void TranslateWidget::updateValue(glm::vec3 distance)
{
    auto mat4socket = _node->getInSockets()[0];
    DoutSocket *cntd = mat4socket->getCntdSocket();
    DNode *transform{nullptr};
    if (cntd && cntd->getNode()->getType() == "TRANSFORM"){
	    transform = cntd->getNode();
    }
    else if (!cntd) {
	    for (auto n : mat4socket->getChildNodes()) {
		    if (n->getType() == "TRANSFORM") {
			    transform = n.get();
			    break;
		    }
	    }
	    mat4socket->setCntdSocket(transform->getOutSockets()[0]);
    }
	auto *socket = transform->getInSockets()[0];
	auto startVal = socket->getProperty().getData<glm::vec3>();

	socket->setProperty(startVal + distance);
}

bool TranslateXWidget::hoverEntered(glm::vec3 p)
{
	return p.x > 1.f && p.x < 2.0f;
}

TranslateXWidget::TranslateXWidget()
    : TranslateWidget(X)
{
	shapegroup_.addShape<Ray>(glm::vec3(1, 0, 0),
							  glm::vec3(1, 0, 0));
}

GL::ShapeRendererGroup* TranslateXWidget::createRenderer()
{
    _renderer = new GL::ArrowRenderer();
    _renderer->setStaticTransformation(glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(0, 0, 1)));
    return _renderer;
}

TranslateYWidget::TranslateYWidget()
    : TranslateWidget(Y)
{
	shapegroup_.addShape<Ray>(glm::vec3(0, 1, 0),
							  glm::vec3(0, 1, 0));
}

GL::ShapeRendererGroup* TranslateYWidget::createRenderer()
{
    _renderer = new GL::ArrowRenderer();
    return _renderer;
}

bool TranslateYWidget::hoverEntered(glm::vec3 p)
{
	return p.y > 1.f && p.y < 2.0f;
}

TranslateZWidget::TranslateZWidget()
    : TranslateWidget(Z)
{
	shapegroup_.addShape<Ray>(glm::vec3(0, 0, 1),
							  glm::vec3(0, 0, 1));
}

GL::ShapeRendererGroup* TranslateZWidget::createRenderer()
{
    _renderer = new GL::ArrowRenderer();
    _renderer->setStaticTransformation(glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1, 0, 0)));
    return _renderer;
}

bool TranslateZWidget::hoverEntered(glm::vec3 p)
{
	return p.z > 1.f && p.z < 2.0f;
}

TranslateXYPlaneWidget::TranslateXYPlaneWidget()
    : TranslateWidget(XY)
{
	shapegroup_.addShape<Rectangle>(glm::vec3(0),
									glm::vec3(1.0, 0, 0),
									glm::vec3(0, 1.0, 0));
}

GL::ShapeRendererGroup* TranslateXYPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();
    auto quad = new GL::QuadRenderer(1.0, 1.0);
    quad->setParentPrimitive(_renderer);

    _renderer->setBorderWidth(3);
    return _renderer;
}

TranslateXZPlaneWidget::TranslateXZPlaneWidget()
    : TranslateWidget(XZ)
{
	shapegroup_.addShape<Rectangle>(glm::vec3(0),
									glm::vec3(1.0, 0, 0),
									glm::vec3(0, 0, 1.0));
}

GL::ShapeRendererGroup* TranslateXZPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();

    auto quad = new GL::QuadRenderer(1.0, 1.0, _renderer);
    quad->setStaticTransformation(glm::rotate(glm::mat4(),
                                              glm::radians(90.f),
                                              glm::vec3(1, 0, 0)));
    _renderer->setBorderWidth(3);

    return _renderer;
}

TranslateYZPlaneWidget::TranslateYZPlaneWidget()
    : TranslateWidget(YZ)
{
	shapegroup_.addShape<Rectangle>(glm::vec3(0),
									glm::vec3(0, 1, 0),
									glm::vec3(0, 0, 1));
}

GL::ShapeRendererGroup* TranslateYZPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();

    auto quad = new GL::QuadRenderer(1.0, 1.0);

    quad->setStaticTransformation(glm::rotate(glm::mat4(),
                                              glm::radians(-90.f),
                                              glm::vec3(0, 1, 0)));
    quad->setParentPrimitive(_renderer);
    _renderer->setBorderWidth(3);

    return _renderer;
}

TranslateScreenPlaneWidget::TranslateScreenPlaneWidget()
    : Widget3D("MAT4")
{
    _screenOriented = true;
    _hoverBorderColor = glm::vec4(1, 1, 0, 1);
    _outBorderColor = glm::vec4(.7, .7, .2, 1);
    _hoverFillColor = glm::vec4(1, 1, 0, .8);
    _outFillColor = glm::vec4(.8, .8, .3, .6);

	shapegroup_.addShape<Rectangle>(glm::vec3(1.75, 1.75, 0),
									glm::vec3(.25, 0, 0),
									glm::vec3(0, .25, 0));
}

GL::ShapeRendererGroup* TranslateScreenPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();
    auto quad = new GL::QuadRenderer(.25, .25, _renderer);
    quad->setStaticTransformation(glm::translate(glm::mat4(), glm::vec3(1.75, 1.75, 0)));
    _renderer->setScreenOriented(true);
    _renderer->setBorderWidth(3);
    return _renderer;
}

void TranslateScreenPlaneWidget::mouseDraged(glm::vec3 point)
{
    if (!_node) return;
    auto transsocket = _node->getInSockets()[1];
    auto startVal = transsocket->getProperty().getData<glm::vec3>();
    auto distance = point - startPoint;

    //transsocket->setProperty(startVal + distance);
    startPoint = point;
}

SwitchWidget::SwitchWidget()
    : Widget3D("MAT4")
{
    _screenOriented = true;
    _hoverBorderColor = glm::vec4(1, 1, 0, 1);
    _outBorderColor = glm::vec4(.7, .7, .2, 1);
    _hoverFillColor = glm::vec4(1, 1, 0, .8);
    _outFillColor = glm::vec4(.8, .8, .3, .6);

	shapegroup_.addShape<Rectangle>(glm::vec3(1.75, 1.75, 0),
									glm::vec3(.25, 0, 0),
									glm::vec3(0, .25, 0));
}

GL::ShapeRendererGroup* SwitchWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();
    auto quad = new GL::QuadRenderer(.25, .25, _renderer);
    quad->setStaticTransformation(glm::translate(glm::mat4(), glm::vec3(1.75, 1.75, 0)));
    _renderer->setScreenOriented(true);
    _renderer->setBorderWidth(3);
    return _renderer;
}
