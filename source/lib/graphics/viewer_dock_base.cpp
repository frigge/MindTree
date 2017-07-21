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

#include "QAction"
#include "QStyle"
#include "QSplitter"
#include "QSizePolicy"
#include "QApplication"
#include "QComboBox"
#include "QVBoxLayout"

#ifndef Q_MOC_RUN
#include "data/signal.h"
#endif

#include "source/graphics/base/mindtree_mainwindow.h"
#include "graphics/windowlist.h"
#include "data/project.h"
#include "data/python/pyutils.h"
#include "viewer.h"
#include "data/windowfactory.h"

#include "viewer_dock_base.h"

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

ViewerDockBase::ViewerDockBase(QString name, AbstractGuiFactory *factory) :
    startSocket(nullptr),
    _factory(factory),
    pinned(false)
{
    setObjectName(name);
    setWindowTitle(name);
    setAttribute(Qt::WA_DeleteOnClose, true);
}

ViewerDockBase::~ViewerDockBase()
{
    if (_factory->getActive() == this)
        _factory->setActive(nullptr);

    MT_CUSTOM_SIGNAL_EMITTER("Viewer_Closed", this);
    auto &openViewers = MindTree::ViewerList::instance()->openViewers;
    std::string viewerID;
    for(auto &p : openViewers) {
        if (p.second == this)
            viewerID = p.first;
    }

    openViewers.erase(viewerID);
}

void ViewerDockBase::focusInEvent(QFocusEvent *event)
{
    QDockWidget::focusInEvent(event);
    _factory->setActive(this);
}

void ViewerDockBase::setViewer(Viewer *view)
{
    viewer_.reset(view);
    setViewerWidget(view->getWidget());
}

void ViewerDockBase::setViewerWidget(QWidget *widget)
{
    auto *w = new QSplitter();
    w->addWidget(widget);

    DNode* settings{nullptr};
    if(viewer_) settings = viewer_->getSettings();
    //setting Node Editor for settings
    if(settings) {
        WId id;
        Python::GILLocker locker;
        try{
            BPy::object module = BPy::import("properties_editor");
            BPy::object settingsObj = utils::getPyObject(settings);
            BPy::object nodeEditorClass = module.attr("Editor");
            editor = nodeEditorClass(settingsObj);
            Py_IncRef(editor.ptr());
            auto pywid = editor.attr("winId")();
            BPy::object builtins = BPy::import("builtins");
            BPy::object intfn = builtins.attr("int");
        
            id = BPy::extract<WId>(intfn(pywid));
        } catch(BPy::error_already_set) {
            PyErr_Print();
        }
        auto *nodeEditorWidget = QWidget::find(id);
        w->addWidget(nodeEditorWidget);
    }
    setWidget(w);
}

Viewer* ViewerDockBase::getViewer()
{
    return viewer_.get();
}

void ViewerDockBase::deleteThis(bool del)
{
    if(!del)deleteLater();
}

void ViewerDockBase::setPythonWidget(BPy::object widget)
{
    WId id;
    try{
        Python::GILLocker locker;
        pywidget = widget();
        auto pywid = pywidget.attr("winId")();
        BPy::object builtins = BPy::import("builtins");
        BPy::object intfn = builtins.attr("int");
        
        id = BPy::extract<WId>(intfn(pywid));
    } catch(BPy::error_already_set) {
        PyErr_Print();
    }
    auto w = QWidget::find(id);
    if(w) setViewerWidget(w);
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
    viewer_->setStart(value);
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
