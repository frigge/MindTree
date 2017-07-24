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
#include "glm/gtx/string_cast.hpp"

#include "QApplication"
#include "QMenu"
#include "QMouseEvent"
#include "QPointF"
#include "QShortcut"

#include "iostream"
#include "ctime"

#include "math.h"

#include "../../3dwidgets/widgets.h"
#include "data/raytracing/ray.h"
#include "data/project.h"

#include "../../render/deferred_renderer.h"
#include "../../datatypes/Object/lights.cpp"
#include  "viewport_widget.h"

#include "data/debuglog.h"

#include "viewport.h"

using namespace MindTree;

std::vector<Viewport*> Viewport::_viewports;

Viewport::Viewport(ViewportWidget *widget) :
    defaultCamera(std::make_shared<Camera>()),
    rotate(false),
    pan(false),
    zoom(false),
    selectionMode(false),
    _showGrid(true),
    transformMode(0),
    _viewportWidget(widget)
{
    _viewports.push_back(this);

    activeCamera = defaultCamera;
    defaultCamera->setFar(1000);

    setMouseTracking(true);

    _widgetManager = std::make_shared<MindTree::Widget3DManager>();
    _renderConfigurator = std::make_unique<GL::DeferredRenderer>(defaultCamera,
                                                                 _widgetManager.get());
}

Viewport::~Viewport()
{
    MindTree::GL::RenderThread::removeManager(_renderConfigurator->getTree());
    _viewports.erase(std::find(begin(_viewports), end(_viewports), this));
}

void Viewport::exitFullscreen()
{
    dbout("exit");
    setWindowState(windowState() & ~Qt::WindowFullScreen);
    _viewportWidget->resetViewport();
    MindTree::GL::RenderThread::update();
}

void Viewport::setOverride(std::string name)
{
    _renderConfigurator->setOverrideOutput(name);
}

void Viewport::clearOverrideOutput()
{
    _renderConfigurator->clearOverrideOutput();
}

GL::RenderTree* Viewport::getRenderTree()
{
    return _renderConfigurator->getManager();
}

void Viewport::setData(std::shared_ptr<Group> value)
{
    if(!value) return;
    _camNames.clear();

    _renderConfigurator->setGeometry(value);

    //cache cameras
    std::unordered_map<std::string, int> camcnt;

    for(const auto &cam : value->getCameras()) {
        std::string name = "Camera";
        if(camcnt.find(name) == end(camcnt))
            camcnt[name] = 0;

        if(camcnt[name] > 0)
            name += std::to_string(camcnt[name]);

        _camNames.push_back(name);
        camcnt[name]++;

        _cameras[name] = cam;
    }

    for(const auto &light : value->getLights()) {
        std::string name = light->getName();
        if(camcnt.find(name) == end(camcnt))
            camcnt[name] = 0;

        if(camcnt[name] > 0)
            name += std::to_string(camcnt[name]);

        _camNames.push_back(name);
        camcnt[name]++;

        _lights[name] = light;
    }

    MindTree::GL::RenderThread::updateOnce();
}

std::vector<std::string> Viewport::getCameras() const
{
    return _camNames;
}

void Viewport::changeCamera(std::string cam)
{
    if(cam == "Default")
    {
        activeCamera = defaultCamera;
    }
    else if(_cameras.find(cam) != end(_cameras)) {
        activeCamera = _cameras[cam];
    }
    else if(_lights.find(cam) != end(_lights)) {
        activeCamera = std::make_shared<Camera>();
        activeCamera->setNear(_lights[cam]->getShadowInfo()._near);
        activeCamera->setFar(_lights[cam]->getShadowInfo()._far);
        activeCamera->setTransformation(_lights[cam]->getWorldTransformation());
        if(_lights[cam]->getLightType() == Light::SPOT)
            activeCamera->setFov(2 * std::dynamic_pointer_cast<SpotLight>(_lights[cam])->getConeAngle());
    }
    else {
        return;
    }

    activeCamera->setAspect((GLdouble)width()/(GLdouble)height());
    activeCamera->setResolution(width(), height());
    _renderConfigurator->setCamera(activeCamera);
}

void Viewport::resizeEvent(QResizeEvent *)
{
    activeCamera->setAspect((GLdouble)width()/(GLdouble)height());
    activeCamera->setResolution(width(), height());
    _renderConfigurator->setCamera(activeCamera);
    MindTree::GL::RenderThread::addManager(_renderConfigurator->getTree(), ctx);
    MindTree::GL::RenderThread::updateOnce();
}

void Viewport::paintEvent(QPaintEvent *)
{
}

void Viewport::showEvent(QShowEvent *)
{
    MindTree::GL::RenderThread::addManager(_renderConfigurator->getTree(), ctx);
}

void Viewport::hideEvent(QHideEvent *)
{
    MindTree::GL::RenderThread::removeManager(_renderConfigurator->getTree());
}

void Viewport::setShowPoints(bool b)
{
    auto config = _renderConfigurator->getManager()->getConfig();
    config.setDrawPoints(b);
    _renderConfigurator->getManager()->setConfig(config);
}

void Viewport::setShowEdges(bool b)
{
    auto config = _renderConfigurator->getManager()->getConfig();
    config.setDrawEdges(b);
    _renderConfigurator->getManager()->setConfig(config);
}

void Viewport::setShowPolygons(bool b)
{
    auto config = _renderConfigurator->getManager()->getConfig();
    config.setDrawPolygons(b);
    _renderConfigurator->getManager()->setConfig(config);
}

void Viewport::setShowFlatShading(bool b)
{
    auto config = _renderConfigurator->getManager()->getConfig();
    config.setShowFlatShaded(b);
    _renderConfigurator->getManager()->setConfig(config);
}

void Viewport::setOption(const std::string &key, Property value)
{
    if(key == "GL:showpoints") setShowPoints(value.getData<bool>());
    if(key == "GL:showedges") setShowEdges(value.getData<bool>());
    if(key == "GL:showpolygons") setShowPolygons(value.getData<bool>());
    if(key == "GL:flatshading") setShowFlatShading(value.getData<bool>());
    else _renderConfigurator->setProperty(key, value);
}

void Viewport::setShowGrid(bool b)
{
    _renderConfigurator->setProperty("GL:showgrid", b);
}

void Viewport::initializeGL()
{
}

void Viewport::resizeGL(int, int)
{
}

void Viewport::paintGL()
{
}

void Viewport::mousePressEvent(QMouseEvent *event)
{
    MindTree::GL::RenderThread::update();
    glm::ivec2 pos;
    pos.x = event->pos().x();
    pos.y = height() - event->pos().y();

    auto viewportSize = glm::ivec2(width(), height());
    if (_widgetManager->mousePressEvent(activeCamera, pos, viewportSize))
        return;

    _renderConfigurator->setProperty("GL:camera:showcenter", true);

    lastpos = event->screenPos();
    glm::vec4 center = _renderConfigurator->getPosition(pos);
    dbout(glm::to_string(center));
    if(center.a > 0) {
        activeCamera->setCenter(center.xyz());
        _renderConfigurator->setProperty("GL:camera:center", center.xyz());
    }

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

    _widgetManager->mouseReleaseEvent();
    _renderConfigurator->setProperty("GL:camera:showcenter", false);
    MindTree::GL::RenderThread::pause();
}

PropertyMap Viewport::getSettings() const
{
    return _renderConfigurator->getSettings();
}

void Viewport::mouseDoubleClickEvent(QMouseEvent *event)
{
    exitFullscreen();
}

void Viewport::mouseMoveEvent(QMouseEvent *event)
{
    glm::ivec2 pos;
    pos.x = event->pos().x();
    pos.y = height() - event->pos().y();

    _renderConfigurator->setProperty("mousePos", pos);

    auto viewportSize = glm::ivec2(width(), height());

    float xdist = lastpos.x()  - event->screenPos().x();
    float ydist = event->screenPos().y() - lastpos.y();
    if(rotate)
        rotateView(xdist, ydist);
    else if(pan)
        panView(xdist, ydist);
    else if(zoom)
        zoomView(xdist, ydist);
    lastpos = event->screenPos();

    if(rotate || pan || zoom)
        return;

    _widgetManager->mouseMoveEvent(activeCamera, pos, viewportSize);
}

void Viewport::wheelEvent(QWheelEvent *event)
{
    zoomView(0, event->delta());
    MindTree::GL::RenderThread::updateOnce();
}

void Viewport::rotateView(float xdist, float ydist)
{
    if(!activeCamera)
        return;
    glm::vec3 campos = activeCamera->getPosition();
    glm::vec3 center = activeCamera->getCenter();
    glm::mat4 camtrans = activeCamera->getTransformation();

    glm::mat4 rotx = glm::rotate(glm::mat4(), glm::radians((float)ydist), glm::cross(glm::vec3(0, 1, 0), -camtrans[2].xyz()));
    glm::mat4 roty = glm::rotate(glm::mat4(), glm::radians((float)xdist), glm::vec3(0, 1, 0));
    glm::mat4 rotation = roty * rotx;

    glm::mat4 translation = glm::translate(glm::mat4(), center);
    camtrans = translation * rotation * glm::inverse(translation) * camtrans;
    activeCamera->setTransformation(camtrans);
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
