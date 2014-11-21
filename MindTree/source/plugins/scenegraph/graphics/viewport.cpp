/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GL/glew.h"

#include "QApplication"
#include "QMenu"
#include "QGraphicsSceneContextMenuEvent"
#include "QMouseEvent"
#include "QGLShaderProgram"
#include "QGLFramebufferObject"
#include "QPointF"
#include "QMetaObject"
#include "GL/glut.h"

#include "iostream"
#include "ctime"

#include "math.h"

#include "../../3dwidgets/widgets.h"
#include "data/raytracing/ray.h"
#include "data/project.h"
#include "../../render/render.h"
#include "../../render/renderpass.h"
#include "../../render/primitive_renderer.h"

#include "viewport.h"

using namespace MindTree;

Viewport::Viewport()
    : QGLWidget(GL::QtContext::getSharedContext()->getNativeContext()), 
    selectedNode(0),
    defaultCamera(std::make_shared<Camera>()),
    rotate(false), 
    pan(false), 
    zoom(false), 
    selectionMode(false), 
    _showGrid(true), 
    transformMode(0)
{
    activeCamera = defaultCamera;
    setAutoBufferSwap(false);

    _rendermanager = std::unique_ptr<GL::RenderManager>(new GL::RenderManager());

    auto pass = _rendermanager->addPass();
    pass->setCamera(activeCamera);

    grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    grid->setTransformation(trans);
    grid->setBorderColor(glm::vec4(.5, .5, .5, .5));
    grid->setAlternatingColor(glm::vec4(.8, .8, .8, .8));
    grid->setBorderWidth(2.);

    pass->addRenderer(grid);
    pass->setDepthOutput(std::make_shared<GL::Texture2D>("depth", GL::Texture::DEPTH));
    pass->addOutput(std::make_shared<GL::Texture2D>("outcolor"));
    pass->addOutput(std::make_shared<GL::Texture2D>("outnormal"));
    pass->addOutput(std::make_shared<GL::Texture2D>("outposition"));

    auto overlaypass = _rendermanager->addPass();
    overlaypass->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
    overlaypass->addOutput(std::make_shared<GL::Texture2D>("overlay"));
    overlaypass->addOutput(std::make_shared<GL::Renderbuffer>("id"));
    overlaypass->addOutput(std::make_shared<GL::Renderbuffer>("position", GL::Renderbuffer::RGBA16F));
    overlaypass->setCamera(activeCamera);

    //testing translate widgets

    Widget3D::insertWidgetsIntoRenderPass(overlaypass);

    _pixelPass = _rendermanager->addPass().get();
    _pixelPass->addRenderer(new GL::FullscreenQuadRenderer());

    setMouseTracking(true);
}

Viewport::~Viewport()
{
    _rendermanager->stop();
}

GL::RenderManager* Viewport::getRenderManager()
{
    return _rendermanager.get();
}

GL::RenderPass* Viewport::getPixelPass()
{
    return _pixelPass;
}

void Viewport::setData(std::shared_ptr<Group> value)
{
    if(!value) return;

    _rendermanager->getPass(0)->setRenderersFromGroup(value);
}

void Viewport::changeCamera(QString cam)    
{
    CameraNode* cnode = (CameraNode*)Project::instance()->getItem(cam.toStdString());
    if(cnode) {
        activeCamera = std::static_pointer_cast<Camera>(cnode->getObject());
        if(selectedNode == cnode)
            selectedNode = 0;
    }
}

void Viewport::resizeEvent(QResizeEvent *)
{
    activeCamera->setProjection((GLdouble)width()/(GLdouble)height());
    _rendermanager->setSize(width(), height());
}

void Viewport::paintEvent(QPaintEvent *)
{
    //do nothing here
}

void Viewport::showEvent(QShowEvent *)
{
    _rendermanager->start();
}

void Viewport::hideEvent(QHideEvent *)
{
    _rendermanager->stop();
}

void Viewport::setShowPoints(bool b)    
{
    auto config = _rendermanager->getConfig();
    config.setDrawPoints(b);
    _rendermanager->setConfig(config);
}

void Viewport::setShowEdges(bool b)    
{
    auto config = _rendermanager->getConfig();
    config.setDrawEdges(b);
    _rendermanager->setConfig(config);
}

void Viewport::setShowPolygons(bool b)    
{
    auto config = _rendermanager->getConfig();
    config.setDrawPolygons(b);
    _rendermanager->setConfig(config);
}

void Viewport::setShowFlatShading(bool b)
{
    auto config = _rendermanager->getConfig();
    config.setShowFlatShaded(b);
    _rendermanager->setConfig(config);
}

void Viewport::setShowGrid(bool b)
{
    grid->setVisible(b);
}

void Viewport::mousePressEvent(QMouseEvent *event)    
{
    glm::ivec2 pos;
    pos.x = event->pos().x();
    pos.y = height() - event->pos().y();

    auto viewportSize = glm::ivec2(width(), height());
    if (Widget3D::mousePressEvent(activeCamera, pos, viewportSize))
        return;

    lastpos = event->posF();
    if(event->modifiers() & Qt::ControlModifier)
        zoom = true;
    else if (event->modifiers() & Qt::ShiftModifier)
        pan = true;
    else
        rotate = true;
}

void Viewport::mouseReleaseEvent(QMouseEvent *event)    
{
    rotate = false;
    pan = false;
    zoom = false;
    selectionMode =false;
    lastpos = QPointF();
    
    Widget3D::mouseReleaseEvent();
}

void Viewport::mouseMoveEvent(QMouseEvent *event)    
{
    glm::ivec2 pos;
    pos.x = event->pos().x();
    pos.y = height() - event->pos().y();

    auto viewportSize = glm::ivec2(width(), height());

    if (Widget3D::mouseMoveEvent(activeCamera, pos, viewportSize))
        return;

    float xdist = lastpos.x()  - event->posF().x();
    float ydist = event->posF().y() - lastpos.y();
    if(rotate)
        rotateView(xdist, ydist);
    else if(pan)
        panView(xdist, ydist);
    else if(zoom)
        zoomView(xdist, ydist);
    lastpos = event->posF();
}

void Viewport::wheelEvent(QWheelEvent *event)    
{
    zoomView(0, event->delta());
}

AbstractTransformableNode* Viewport::getSelectedNode()    
{
    if(selectedNode && selectedNode->getObject() == activeCamera)
        return 0;
    else return selectedNode;
}

glm::vec3 Viewport::getCamPivot()    
{
    if(getSelectedNode())
        return glm::vec4(getSelectedNode()->getObject()->getTransformation() * glm::vec4()).xyz();
    else
        return glm::vec3(0, 0, 0);
}

void Viewport::rotateView(float xdist, float ydist)
{
    if(!activeCamera)
        return;
    glm::mat4 mat;
    glm::vec3 campos = activeCamera->getPosition();
    glm::vec3 center = activeCamera->getCenter();
    glm::vec3 lookat = campos - center;

    mat = glm::rotate(mat, (float)ydist, glm::cross(glm::vec3(0, 1, 0), center - campos));
    mat = glm::rotate(mat, (float)xdist, glm::vec3(0, 1, 0));
    lookat = (mat * glm::vec4(lookat, 1)).xyz();
    activeCamera->posAroundCenter(lookat + center);
}

void Viewport::panView(float xdist, float ydist)    
{
    if(!activeCamera)
        return;
    float lalen = glm::length(activeCamera->getCenter() - activeCamera->getPosition());
    float xtrans = xdist * lalen * 0.005;
    float ytrans = ydist * lalen * 0.005;
    activeCamera->translate(glm::vec3(1, 0, 0) * xtrans);
    activeCamera->translate(glm::vec3(0, 1, 0) * ytrans);
}

void Viewport::zoomView(float xdist, float ydist)    
{
    if(!activeCamera)
        return;
    //glm::vec3 zvec = activeCamera->getCenter() - activeCamera->getPosition();
    activeCamera->moveToCenter(ydist/height());
}
