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

#include "QComboBox"
#include "QWidgetAction"

#include "iostream"
#include "viewport.h"
#include "data/nodes/data_node.h"
#include "../../render/rendertree.h"
#include "../../render/renderpass.h"
#include "../../render/render.h"
#include "data/debuglog.h"

#include "viewport_widget.h"

using namespace MindTree;
ViewportViewer::ViewportViewer(DoutSocket *socket)
    : Viewer(socket)
{
    setWidget(new ViewportWidget(this));
}

ViewportViewer::~ViewportViewer()
{
}

void ViewportViewer::update()
{
    auto widget = dynamic_cast<ViewportWidget*>(getWidget());
    auto *viewport = widget->getViewport();
    Property data = cache.getOutput(getStart());

    if(data.getType() == "GROUPDATA") {
        viewport->setData(data.getData<GroupPtr>());
    }
    else if(data.getType() == "TRANSFORMABLE") {
        auto obj = data.getData<AbstractTransformablePtr>();
        assert(obj);
        auto grp = std::make_shared<Group>();
        grp->addMember(obj);
        viewport->setData(grp);
    }
    widget->setCameras();
}

ViewportWidget::ViewportWidget(ViewportViewer *viewer)
    : _viewport(new Viewport(this)), _viewer(viewer)
{
    createMainLayout();
    createToolbar();
}

ViewportWidget::~ViewportWidget()
{
    delete _viewer;
}

void ViewportWidget::setCameras()
{
    auto cameras = _viewport->getCameras();

    //try to cache last camera
    QString lastcam = _camBox->currentText();
    _camBox->clear();
    QStringList list;
    list.append("Default");
    for(const auto &cam : cameras) {
        list.append(cam.c_str());
    }

    _camBox->addItems(list);

    int itemIndex = _camBox->findText(lastcam);
    _camBox->setCurrentIndex(itemIndex);
}

Viewport* ViewportWidget::getViewport()    
{
    return _viewport;
}

QSize ViewportWidget::sizeHint()    const
{
    return QSize(400, 800); 
}

void ViewportWidget::createMainLayout()    
{
    auto mainlayout = new QVBoxLayout();
    setLayout(mainlayout);
    _tools = new QToolBar;
    mainlayout->addWidget(_tools);
    mainlayout->addWidget(_viewport);
}

void ViewportWidget::resetViewport()
{
    layout()->addWidget(_viewport);
}

void ViewportWidget::createToolbar()    
{
    QAction *showPointsAction = _tools->addAction("Show Points");
    showPointsAction->setCheckable(true);
    showPointsAction->setChecked(true);

    QAction *showEdgesAction = _tools->addAction("Show Edges");
    showEdgesAction->setCheckable(true);
    showEdgesAction->setChecked(true);

    QAction *showPolygonsAction = _tools->addAction("Show Faces");
    showPolygonsAction->setCheckable(true);
    showPolygonsAction->setChecked(true);

    QAction *toggleDefaultLight = _tools->addAction("Default Light");
    toggleDefaultLight->setCheckable(true);
    toggleDefaultLight->setChecked(true);

    QAction *showFlatShadedAction = _tools->addAction("Flat Shaded");
    showFlatShadedAction->setCheckable(true);
    showFlatShadedAction->setChecked(false);

    QAction *showGridAction = _tools->addAction("Grid");
    showGridAction->setCheckable(true);
    showGridAction->setChecked(true);

    QAction *fullscreenAction = _tools->addAction("Fullscreen");

    QAction *overrideOutputAction = _tools->addAction("Override Output");
    overrideOutputAction->setCheckable(true);
    overrideOutputAction->setChecked(false);

    auto outputs = _viewport->getRenderTree()->getAllOutputs();
    _outputBox = new QComboBox();
    _outputBox->setEnabled(false);

    QStringList l;
    for(auto out : outputs)
        l.append(out.c_str());

    _outputBox->addItems(l);

    _camBox = new QComboBox();
    auto *camboxAction = new QWidgetAction(_tools);
    auto *outputBoxAction = new QWidgetAction(_tools);
    camboxAction->setDefaultWidget(_camBox);
    outputBoxAction->setDefaultWidget(_outputBox);
    _tools->addAction(camboxAction);
    _tools->addAction(outputBoxAction);

    connect(_outputBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setOutput(QString)));
    connect(_camBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setCamera(QString)));

    connect(showPointsAction, SIGNAL(toggled(bool)), this, SLOT(togglePoints(bool)));
    connect(showEdgesAction, SIGNAL(toggled(bool)), this, SLOT(toggleEdges(bool)));
    connect(showPolygonsAction, SIGNAL(toggled(bool)), this, SLOT(togglePolygons(bool)));
    connect(showFlatShadedAction, SIGNAL(toggled(bool)), this, SLOT(toggleFlatShading(bool)));
    connect(showGridAction, SIGNAL(toggled(bool)), this, SLOT(toggleGrid(bool)));
    connect(toggleDefaultLight, SIGNAL(toggled(bool)), this, SLOT(toggleDefaultLighting(bool)));
    connect(overrideOutputAction, SIGNAL(toggled(bool)), this, SLOT(setOverrideOutput(bool)));
    connect(fullscreenAction, SIGNAL(triggered(bool)), this, SLOT(setFullscreen()));
}

void ViewportWidget::setFullscreen()
{
    _viewport->setParent(nullptr);
    _viewport->setWindowState(_viewport->windowState() | Qt::WindowFullScreen);
    _viewport->show();
}

void ViewportWidget::setCamera(QString cam)
{
    _viewport->changeCamera(cam.toStdString());
}

void ViewportWidget::setOverrideOutput(bool value)
{
    _outputBox->setEnabled(value);
    if(!value) {
        _viewport->clearOverrideOutput();
    }
}

void ViewportWidget::setOutput(QString out)
{
    _viewport->clearOverrideOutput();
    _viewport->setOverride(out.toStdString());
}

void ViewportWidget::toggleDefaultLighting(bool value)
{
    _viewport->setOption("defaultLighting", value);
}

void ViewportWidget::togglePoints(bool b)    
{
    _viewport->setShowPoints(b);
}

void ViewportWidget::toggleEdges(bool b)    
{
    _viewport->setShowEdges(b);
}

void ViewportWidget::togglePolygons(bool b)    
{
    _viewport->setShowPolygons(b);
}

void ViewportWidget::toggleFlatShading(bool b)
{
    _viewport->setShowFlatShading(b);
}

void ViewportWidget::toggleGrid(bool b)
{
    _viewport->setShowGrid(b);
}
