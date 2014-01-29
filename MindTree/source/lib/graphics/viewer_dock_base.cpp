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

#include "viewer_dock_base.h"

#include "QAction"
#include "QStyle"
#include "QSizePolicy"
#include "QApplication"
#include "QComboBox"

#include "data/signal.h"

#include "data/frg.h"
#include "source/graphics/base/mindtree_mainwindow.h"
#include "viewer.h"

using namespace MindTree;

ViewerTitleBarButton::ViewerTitleBarButton(ViewerDockHeaderWidget *widget)
{
    //setFlat(true);
    widget->addWidget(this);
    setDefaultAction(new QAction("", 0));
}

QSize ViewerTitleBarButton::sizeHint()    const
{
    return QSize(15, 15);
}

ViewerDockHeaderWidget::ViewerDockHeaderWidget(ViewerDockBase *viewer)
    : viewer(viewer), viewerBox(0)
{
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(spacer);
    //createViewerBox();
    //QAction *pinAction = addAction("");
    //QAction *detachAction = addAction("");
    //detachAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    //QAction *closeAction = addAction("");
    //closeAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    createButtons();
    //connect(FRG::Space, SIGNAL(nodeDroped(DNode*)), this, SLOT(addToViewerBox(DNode*)));
    //connect(viewerBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(getViewerNode(QString)));
}

void ViewerDockHeaderWidget::fillViewerBox()
{
    //std::list<DNode*> vList = FRG::CurrentProject->getViewers();
    //std::list<DNode*>::iterator it;
    //for(it=vList.begin(); it!=vList.end(); it++)
    //    viewerBox->addItem((*it)->getNodeName());
    //if(!vList.empty())
    //    getViewerNode(viewerBox->currentText());
}

void ViewerDockHeaderWidget::addToViewerBox(DNode *node)    
{
    //if(!DNode::isViewerNode(node))
    //    return;
    if(!comboBoxContains(node->getNodeName().c_str())) viewerBox->addItem(node->getNodeName().c_str());
}

bool ViewerDockHeaderWidget::comboBoxContains(QString name)    
{
    for(int i=0; i<viewerBox->count(); i++)
        if(viewerBox->itemText(i)==name) return true;
    return false;
}

void ViewerDockHeaderWidget::createViewerBox()    
{
    viewerBox = new QComboBox(this);
    fillViewerBox();
}

void ViewerDockHeaderWidget::createButtons()    
{
    pinButton = new ViewerTitleBarButton(this);
    pinButton->setCheckable(true);
    ViewerTitleBarButton *detachButton = new ViewerTitleBarButton(this);
    ViewerTitleBarButton *closeButton = new ViewerTitleBarButton(this);
    detachButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(detachButton->defaultAction(), SIGNAL(triggered()), viewer, SLOT(toggleFloating()));
    connect(closeButton->defaultAction(), SIGNAL(triggered()), viewer, SLOT(destroy()));
    connect(pinButton->defaultAction(), SIGNAL(triggered(bool)), viewer, SLOT(setPinned(bool)));
}

QSize ViewerDockHeaderWidget::sizeHint()    const
{
    return QSize(20, 20);
}

ViewerDockBase::ViewerDockBase(QString name)
    : startSocket(0), pinned(false), viewer(0)
{
    auto n = FRG::CurrentProject->registerItem(this, name);
    setObjectName(n);
    setWindowTitle(n);
    //setTitleBarWidget(new ViewerDockHeaderWidget(this));
    //FRG::Author->addDockWidget(Qt::RightDockWidgetArea, this);
    //connect(FRG::Space, SIGNAL(selectionChanged()), this, SLOT(updateFocus()));
    //connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(adjust(bool)));
    //connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(deleteThis(bool)));
}

ViewerDockBase::~ViewerDockBase()
{
    FRG::CurrentProject->unregisterItem(this);
    MT_CUSTOM_SIGNAL_EMITTER("Viewer_Closed", this);
}

void ViewerDockBase::setViewer(Viewer *view)    
{
    viewer = view;
    setWidget(view->getWidget());
}

void ViewerDockBase::deleteThis(bool del)    
{
    if(!del)deleteLater();
}

void ViewerDockBase::setPythonWidget(BPy::object widget)    
{
    WId id;
    try{
        pywidget = widget();
        id = BPy::extract<WId>(pywidget.attr("winId")());
    } catch(BPy::error_already_set) {
        PyErr_Print();
    }
    auto w = QWidget::find(id);
    if(w) setWidget(w);
    else std::cout<<"Window not valid"<<std::endl;
}

void ViewerDockBase::init()    
{
}

void ViewerDockBase::setPinned(bool pin)    
{
    pinned = pin;
}

void ViewerDockBase::adjust(bool vis)    
{
    //if(vis) {
    //    connect((QObject*)FRG::Space, SIGNAL(linkChanged()), this, SLOT(update()));
    //    connect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), this, SLOT(update(DNode*)));
    //}
    //else {
    //    disconnect((QObject*)FRG::Space, SIGNAL(linkChanged()), this, SLOT(update()));
    //    disconnect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), this, SLOT(update(DNode*)));
    //}
}

void ViewerDockBase::updateFocus()    
{
    //if(pinned) return;
    //if(!FRG::Space->selectedNodes().isEmpty()) {
    //    DNode *n = FRG::Space->selectedNodes().first();
    //    if(DNode::isViewerNode(n)) {
    //        setFocusNode(node);
    //    }
    //}
}

DoutSocket* ViewerDockBase::getStart()
{
    return startSocket;
}

void ViewerDockBase::setStart(DoutSocket* value)
{
    startSocket = value;
}

void ViewerDockBase::setFocusNode(DNode* node)    
{
    this->node = node;
}

void ViewerDockBase::update()    
{
    exec(); 
}

void ViewerDockBase::update(DNode* node)    
{
    exec(node);
}

void ViewerDockBase::toggleFloating()    
{
    setFloating(!isFloating());
}

void ViewerDockBase::destroy()    
{
    delete this;
}

