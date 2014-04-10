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
#include "data/frg.h"
#include "data/nodes/data_node.h"

using namespace MindTree;
ViewportViewer::ViewportViewer(DoutSocket *socket)
    : Viewer(socket)
{
    setWidget(new ViewportWidget(this));
}

ViewportViewer::~ViewportViewer()
{
}

void ViewportViewer::update(DinSocket *)
{
    auto *viewport = static_cast<ViewportWidget*>(getWidget())->getViewport();
    Property data = cache.getData(0);

    if(data.getType() == "GROUPDATA") {
        viewport->setData(data.getData<GroupPtr>());
    }
    else if(data.getType() == "SCENEOBJECT") {
        auto obj = data.getData<GeoObjectPtr>();
        auto grp = std::make_shared<Group>();
        grp->addMember(obj);
        viewport->setData(grp);
    }
}

ViewportWidget::ViewportWidget(ViewportViewer *viewer)
    : viewport(new Viewport()), viewer(viewer)
{
    createMainLayout();
    createToolbar();
}

ViewportWidget::~ViewportWidget()
{
    delete viewer;
}

Viewport* ViewportWidget::getViewport()    
{
    return viewport;
}

QSize ViewportWidget::sizeHint()    const
{
    return QSize(400, 800); 
}

void ViewportWidget::createMainLayout()    
{
    auto mainlayout = new QVBoxLayout();
    setLayout(mainlayout);
    tools = new QToolBar;
    mainlayout->addWidget(tools);
    mainlayout->addWidget(viewport);
}

void ViewportWidget::createToolbar()    
{
    QAction *showPointsAction = tools->addAction("P");
    showPointsAction->setCheckable(true);
    showPointsAction->setChecked(true);

    QAction *showEdgesAction = tools->addAction("E");
    showEdgesAction->setCheckable(true);
    showEdgesAction->setChecked(true);

    QAction *showPolygonsAction = tools->addAction("F");
    showPolygonsAction->setCheckable(true);
    showPolygonsAction->setChecked(true);

    QAction *showFlatShadedAction = tools->addAction("Flat Shaded");
    showFlatShadedAction->setCheckable(true);
    showFlatShadedAction->setChecked(false);

    camBox = new QComboBox();
    tools->addWidget(camBox);
    //camBox->connect(dock->getViewport(), SIGNAL(sceneUpdated()), this, SLOT(refillCamBox()));
    //camBox->connect(camBox, SIGNAL(currentIndexChanged(QString)), dock->getViewport(), SLOT(changeCamera(QString)));

    connect(showPointsAction, SIGNAL(toggled(bool)), this, SLOT(togglePoints(bool)));
    connect(showEdgesAction, SIGNAL(toggled(bool)), this, SLOT(toggleEdges(bool)));
    connect(showPolygonsAction, SIGNAL(toggled(bool)), this, SLOT(togglePolygons(bool)));
    connect(showFlatShadedAction, SIGNAL(toggled(bool)), this, SLOT(toggleFlatShading(bool)));
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
    viewport->setShowPoints(b);
}

void ViewportWidget::toggleEdges(bool b)    
{
    viewport->setShowEdges(b);
}

void ViewportWidget::togglePolygons(bool b)    
{
    viewport->setShowPolygons(b);
}

void ViewportWidget::toggleFlatShading(bool b)
{
    viewport->setShowFlatShading(b);
}
