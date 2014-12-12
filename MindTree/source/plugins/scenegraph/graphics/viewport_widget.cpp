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

#include "viewport_widget.h"

#include "QComboBox"

#include "iostream"
#include "viewport.h"
#include "data/nodes/data_node.h"
#include "../../render/rendermanager.h"
#include "../../render/renderpass.h"
#include "../../render/render.h"

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
    auto *viewport = static_cast<ViewportWidget*>(getWidget())->getViewport();
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
}

ViewportWidget::ViewportWidget(ViewportViewer *viewer)
    : _viewport(new Viewport()), _viewer(viewer)
{
    createMainLayout();
    createToolbar();
}

ViewportWidget::~ViewportWidget()
{
    delete _viewer;
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

void ViewportWidget::createToolbar()    
{
    QAction *showPointsAction = _tools->addAction("P");
    showPointsAction->setCheckable(true);
    showPointsAction->setChecked(true);

    QAction *showEdgesAction = _tools->addAction("E");
    showEdgesAction->setCheckable(true);
    showEdgesAction->setChecked(true);

    QAction *showPolygonsAction = _tools->addAction("F");
    showPolygonsAction->setCheckable(true);
    showPolygonsAction->setChecked(true);

    QAction *showFlatShadedAction = _tools->addAction("Flat Shaded");
    showFlatShadedAction->setCheckable(true);
    showFlatShadedAction->setChecked(false);

    QAction *showGridAction = _tools->addAction("Grid");
    showGridAction->setCheckable(true);
    showGridAction->setChecked(true);

    auto outputs = _viewport->getRenderManager()->getAllOutputs();
    auto *outputBox = new QComboBox();

    QStringList l;
    for(auto out : outputs)
        l.append(out.c_str());

    outputBox->addItems(l);

    _camBox = new QComboBox();
    _tools->addWidget(_camBox);
    _tools->addWidget(outputBox);

    connect(outputBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setOutput(QString)));

    connect(showPointsAction, SIGNAL(toggled(bool)), this, SLOT(togglePoints(bool)));
    connect(showEdgesAction, SIGNAL(toggled(bool)), this, SLOT(toggleEdges(bool)));
    connect(showPolygonsAction, SIGNAL(toggled(bool)), this, SLOT(togglePolygons(bool)));
    connect(showFlatShadedAction, SIGNAL(toggled(bool)), this, SLOT(toggleFlatShading(bool)));
    connect(showGridAction, SIGNAL(toggled(bool)), this, SLOT(toggleGrid(bool)));
}

void ViewportWidget::setOutput(QString out)
{
    _viewport->getRenderManager()->clearCustomTextureNameMapping();
    _viewport->getRenderManager()->setCustomTextureNameMapping(out.toStdString(), "outcolor");
}

void ViewportWidget::refillCamBox()    
{
    //QString curCamName = camBox->currentText();
    //camBox->clear();
    ////DoutSocket *startSocket = dock->getViewport()->getStartSocket();
    //DoutSocket *startSocket = 0;
    //if(!startSocket)return;
    //auto cameras = grp->getCameras();
    //for(auto cam : cameras)
    //    camBox->addItem(cam->getName().c_str());
    //if(camBox->findData(curCamName) == -1)
    //    camBox->setCurrentIndex(camBox->findText(curCamName));
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
