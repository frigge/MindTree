#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "data/nodes/data_node.h"
#include "translate_widgets.h"

using namespace MindTree;

TranslateXWidget::TranslateXWidget()
    : TranslateWidget(X)
{
    auto hitrect = std::make_shared<Rectangle>(glm::vec3(1, -0.05, 0), 
                                               glm::vec3(1.0, 0, 0), 
                                               glm::vec3(0, 0.1, 0));
    addShape(hitrect);
    hitrect = std::make_shared<Rectangle>(glm::vec3(1, 0, -0.05), 
                                          glm::vec3(1.0, 0, 0), 
                                          glm::vec3(0, 0.0, .1));
    addShape(hitrect);
}

TranslateXWidget::~TranslateXWidget()
{
}

GL::ShapeRendererGroup* TranslateXWidget::createRenderer()
{
    _renderer = new GL::ArrowRenderer();
    _renderer->setStaticTransformation(glm::rotate(glm::mat4(), -90.0f, glm::vec3(0, 0, 1)));
    return _renderer;
}

TranslateYWidget::TranslateYWidget()
    : TranslateWidget(Y)
{
    auto hitrect = std::make_shared<Rectangle>(glm::vec3(-0.05, 1, 0), 
                                               glm::vec3(0.1, 0, 0), 
                                               glm::vec3(0, 1., 0));
    addShape(hitrect);
    hitrect = std::make_shared<Rectangle>(glm::vec3(0, 1, -0.05), 
                                          glm::vec3(0.0, 0, 0.1), 
                                          glm::vec3(0, 1, 0));
    addShape(hitrect);
}

TranslateYWidget::~TranslateYWidget()
{
}

GL::ShapeRendererGroup* TranslateYWidget::createRenderer()
{
    _renderer = new GL::ArrowRenderer();
    return _renderer;
}

TranslateZWidget::TranslateZWidget()
    : TranslateWidget(Z)
{
    auto hitrect = std::make_shared<Rectangle>(glm::vec3(-0.05, 0, 1), 
                                               glm::vec3(0, 0, 1), 
                                               glm::vec3(0.1, 0, 0));
    addShape(hitrect);
    hitrect = std::make_shared<Rectangle>(glm::vec3(0, -0.05, 1), 
                                          glm::vec3(0, 0.1, 0), 
                                          glm::vec3(0, 0, 1));
    addShape(hitrect);
}

TranslateZWidget::~TranslateZWidget()
{
}

GL::ShapeRendererGroup* TranslateZWidget::createRenderer()
{
    _renderer = new GL::ArrowRenderer();
    _renderer->setStaticTransformation(glm::rotate(glm::mat4(), 90.0f, glm::vec3(1, 0, 0)));
    return _renderer;
}

TranslateXYPlaneWidget::TranslateXYPlaneWidget()
    : TranslateWidget(XY)
{
    auto hitrect = std::make_shared<Rectangle>(glm::vec3(0), 
                                               glm::vec3(1.0, 0, 0), 
                                               glm::vec3(0, 1.0, 0));
    addShape(hitrect);
}

TranslateXYPlaneWidget::~TranslateXYPlaneWidget()
{
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
    auto hitrect = std::make_shared<Rectangle>(glm::vec3(0), 
                                               glm::vec3(1.0, 0, 0), 
                                               glm::vec3(0, 0, 1.0));
    addShape(hitrect);
}

TranslateXZPlaneWidget::~TranslateXZPlaneWidget()
{
}

GL::ShapeRendererGroup* TranslateXZPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();

    auto quad = new GL::QuadRenderer(1.0, 1.0);
    quad->setStaticTransformation(glm::rotate(glm::mat4(), 
                                              90.f, 
                                              glm::vec3(1, 0, 0)));
    quad->setParentPrimitive(_renderer);
    _renderer->setBorderWidth(3);
    
    return _renderer;
}

TranslateYZPlaneWidget::TranslateYZPlaneWidget()
    : TranslateWidget(YZ)
{
    auto hitrect = std::make_shared<Rectangle>(glm::vec3(0), 
                                               glm::vec3(0, 1, 0), 
                                               glm::vec3(0, 0, 1));
    addShape(hitrect);
}

TranslateYZPlaneWidget::~TranslateYZPlaneWidget()
{
}

GL::ShapeRendererGroup* TranslateYZPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();

    auto quad = new GL::QuadRenderer(1.0, 1.0);

    quad->setStaticTransformation(glm::rotate(glm::mat4(), 
                                              -90.f, 
                                              glm::vec3(0, 1, 0)));
    quad->setParentPrimitive(_renderer);
    _renderer->setBorderWidth(3);
    
    return _renderer;
}

TranslateScreenPlaneWidget::TranslateScreenPlaneWidget()
    : Widget3D("TRANSFORM")
{
    _screenOriented = true;
    _hoverBorderColor = glm::vec4(1, 1, 0, 1);
    _outBorderColor = glm::vec4(.7, .7, .2, 1);
    _hoverFillColor = glm::vec4(1, 1, 0, .8);
    _outFillColor = glm::vec4(.8, .8, .3, .6);

    auto hitrect = std::make_shared<Rectangle>(glm::vec3(1.75, 1.75, 0), 
                                               glm::vec3(.25, 0, 0), 
                                               glm::vec3(0, .25, 0)); 

    addShape(hitrect);
}

GL::ShapeRendererGroup* TranslateScreenPlaneWidget::createRenderer()
{
    _renderer = new GL::ShapeRendererGroup();
    auto quad = new GL::QuadRenderer(.25, .25);
    quad->setStaticTransformation(glm::translate(glm::mat4(), glm::vec3(1.75, 1.75, 0)));
    quad->setParentPrimitive(_renderer);
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

    transsocket->setProperty(startVal + distance);
    startPoint = point;
}
