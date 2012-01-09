/*
    frg_Shader_Author Shader Editor, a Node-based Renderman Shading Language Editor
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

#include "frg_shader_author.h"

#include "QtGui"
#include "QGraphicsItem"
#include "QGraphicsView"
#include "QGraphicsScene"
#include "QPainterPath"
#include "QWheelEvent"
#include "QPalette"
#include "QMenu"
#include "QDockWidget"

#include "QDir"
#include "iostream"

#include "math.h"

#include "source/data/base/frg.h"
#include "source/data/base/dnspace.h"
#include "source/data/nodes/data_node.h"
#include "source/graphics/shader_view.h"
#include "source/graphics/shaderpreview.h"
#include "source/graphics/previewdock.h"
#include "source/graphics/properties_editor.h"
#include "source/graphics/detailsview.h"
#include "source/data/data_info_box.h"
#include "source/graphics/vis_info_box.h"

ChangeSpaceAction::ChangeSpaceAction(DNSpace *space, QObject *parent)
    : QAction(parent)
{
    new_space = space;
    connect(this, SIGNAL(triggered()), this, SLOT(emitTriggered()));
}

void ChangeSpaceAction::emitTriggered()
{
    emit triggered(new_space);
}


frg_Shader_Author::frg_Shader_Author(QWidget *parent)
    : QMainWindow(parent)
{
    FRG::Author = this;
    setWindowTitle("frg Shader Author");
    resize(1500, 800);

    setDockNestingEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    createSpaceDock();
    toolbar = addToolBar("Node Path");
    updateToolBar();
    createMenus();
    createDocks();
    FRG::previewEditor = new PreviewSceneEditor();
    FRG::previewEditor->hide();

    statusBar()->showMessage("Welcome to frg Shader Author");
	newfile();
};

void frg_Shader_Author::createSpaceDock()    
{
	FRG::Space = new VNSpace();

    view = new Shader_View(this);

    view->setScene(FRG::Space);
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setUpdatesEnabled(true);

    spaceDock = new QDockWidget("Node Space");
    spaceDock->setWidget(view);
    addDockWidget(Qt::RightDockWidgetArea, spaceDock);
}

void frg_Shader_Author::createDocks()    
{
    nodeeditor = new QDockWidget("Node Editor");
    nodeeditor->setWidget(new NewNodeEditor);
    addDockWidget(Qt::TopDockWidgetArea, nodeeditor);    
    nodeeditor->hide();

    nodelib = new QDockWidget("Node Library");
    nodelib->setWidget(new NodeLibWidget(this));
    addDockWidget(Qt::LeftDockWidgetArea, nodelib); 

    propeditor = new QDockWidget("Properties Editor");
    propeditor->setWidget(new PropertiesEditor(this));
    addDockWidget(Qt::LeftDockWidgetArea, propeditor);

    detailsview = new QDockWidget("Details View");
    detailsview->setWidget(new DetailsView(this));
    addDockWidget(Qt::RightDockWidgetArea, detailsview);
    tabifyDockWidget(detailsview, spaceDock);
}

void frg_Shader_Author::createPreviewDock(DShaderPreview *prev)    
{
    PreviewDock *prevDock = new PreviewDock(prev);
    addDockWidget(Qt::RightDockWidgetArea, prevDock);
}

void frg_Shader_Author::moveIntoSpace(DNSpace *space)
{
    FRG::Space->moveIntoSpace(space);

    while(spaces.contains(space))
        spaces.takeLast();
    spaces.append(space);
    updateToolBar();
}

void frg_Shader_Author::updateToolBar()
{
    foreach(QAction *action, toolbar->actions())
        toolbar->removeAction(action);

    QAction *rootAction = toolbar->addAction("/Root");
    connect(rootAction, SIGNAL(triggered()), this, SLOT(setRoot_Space()));

    while(!spaceActions.isEmpty())
        delete spaceActions.takeLast();

    foreach(DNSpace *space, spaces)
    {
        ChangeSpaceAction *chspace = new ChangeSpaceAction(space, toolbar);
        spaceActions.append(chspace);
        chspace->setText("/" + space->getName());
        toolbar->addAction(chspace);
        connect(chspace, SIGNAL(triggered(DNSpace*)), this, SLOT(moveIntoSpace(DNSpace*)));
    }
}

void frg_Shader_Author::setRoot_Space()
{
    FRG::CurrentProject->moveIntoRootSpace();
    spaces.clear();
    updateToolBar();
}

void frg_Shader_Author::createMenus()
{
    QMenu *fileMenu = new QMenu("&File");
    QAction *newAction = fileMenu->addAction("&New");
    newAction->setShortcuts(QKeySequence::New);
    QAction *openAction = fileMenu->addAction("&Open");
    openAction->setShortcuts(QKeySequence::Open);
    QAction *saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcuts(QKeySequence::Save);
    QAction *saveAsAction = fileMenu->addAction("Save &As");
    saveAsAction->setShortcuts(QKeySequence::SaveAs);
    QAction *quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcuts(QKeySequence::Quit);

    QMenu *editMenu = new QMenu("&Edit");
    QAction *undoAction = editMenu->addAction("&Undo");
    undoAction->setShortcuts(QKeySequence::Undo);
    QAction *redoAction = editMenu->addAction("&Redo");
    redoAction->setShortcuts(QKeySequence::Redo);
    QAction *deleteAction = editMenu->addAction("&Delete");
    deleteAction->setShortcuts(QKeySequence::Delete);
    QAction *copyAction = editMenu->addAction("&Copy");
    copyAction->setShortcuts(QKeySequence::Copy);
    QAction *cutAction = editMenu->addAction("C&ut");
    cutAction->setShortcuts(QKeySequence::Cut);
    QAction *pasteAction = editMenu->addAction("&Paste");
    pasteAction->setShortcuts(QKeySequence::Paste);

    QMenu *nodeMenu = new QMenu("&Nodes");
    QAction *infoBoxAction = nodeMenu->addAction("&Info Box");
    QAction *containerAction = nodeMenu->addAction("&Build Container");
    containerAction->setShortcut(QString("c"));
    QAction *unpackAction = nodeMenu->addAction("&Unpack Container");
    unpackAction->setShortcut(QString("Alt+c"));
    QAction *nodeEditorAction = nodeMenu->addAction("&New Node");
    nodeEditorAction->setShortcut(QString("n"));
    nodeEditorAction->setCheckable(true);
    QAction *nodeLibAction = nodeMenu->addAction("&Node Library");
    nodeLibAction->setCheckable(true);
    nodeLibAction->setChecked(true);
    nodeLibAction->setShortcut(QString("l"));

    QMenu *previewMenu = new QMenu("&Preview");
    QAction *editprevAction = previewMenu->addAction("&Edit Previews");
    connect(editprevAction, SIGNAL(triggered()), this, SLOT(openPreviewEditor()));

    connect(newAction, SIGNAL(triggered()), this, SLOT(newfile()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(removeNodes()));
    connect(copyAction, SIGNAL(triggered()), FRG::Space, SLOT(copy()));
    connect(cutAction, SIGNAL(triggered()), FRG::Space, SLOT(cut()));
    connect(pasteAction, SIGNAL(triggered()), FRG::Space, SLOT(paste()));

    connect(infoBoxAction, SIGNAL(triggered()), this, SLOT(addInfoBox()));
    connect(containerAction, SIGNAL(triggered()), this, SLOT(buildContainer()));
    connect(unpackAction, SIGNAL(triggered()), this, SLOT(unpackContainer()));
    connect(nodeEditorAction, SIGNAL(triggered()), this, SLOT(toggleNodeEditor()));
    connect(nodeLibAction, SIGNAL(triggered()), this, SLOT(toggleNodeLib()));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(editMenu);
    menuBar()->addMenu(nodeMenu);
    menuBar()->addMenu(previewMenu);
};

void frg_Shader_Author::addInfoBox()    
{
    DInfoBox *infoBox = new DInfoBox();
    infoBox->getVis();
}

void frg_Shader_Author::toggleNodeLib()    
{
    if(nodelib->isVisible())
        nodelib->hide();
    else
        nodelib->show();
}

void frg_Shader_Author::toggleNodeEditor()    
{
    if(nodeeditor->isVisible())
        nodeeditor->hide();
    else
        nodeeditor->show();
}

void frg_Shader_Author::openPreviewEditor()    
{
    FRG::previewEditor->show();
}

void frg_Shader_Author::removeNodes()    
{
    NodeList nodes = FRG::Space->selectedNodes();
    FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoRemoveNode(nodes));
    FRG::Space->removeNode(nodes);
}

void frg_Shader_Author::buildContainer()    
{
    FRG::Space->buildContainer();
}

void frg_Shader_Author::unpackContainer()    
{
    FRG::Space->unpackContainer(); 
}

void frg_Shader_Author::change_window_title(QString title)
{
    QString newtitle("frg Shader Author");
    newtitle.append(" - ");
    newtitle.append(title);
    setWindowTitle(newtitle);
}

void frg_Shader_Author::newfile()
{
	delete FRG::CurrentProject;
    new Project();
    change_window_title("");
}

void frg_Shader_Author::save()
{
	FRG::CurrentProject->save();
	change_window_title(FRG::CurrentProject->getFilename());
}

void frg_Shader_Author::saveAs()
{
	FRG::CurrentProject->saveAs();
    change_window_title(FRG::CurrentProject->getFilename());
}

void frg_Shader_Author::open()
{
    QString name = QFileDialog::getOpenFileName();
    delete FRG::CurrentProject;
    new Project(name);
	change_window_title(name);	
}

void frg_Shader_Author::undo()    
{
    FRG::SpaceDataInFocus->undo();
}

void frg_Shader_Author::redo()    
{
    FRG::SpaceDataInFocus->redo();
}

frg_Shader_Author::~frg_Shader_Author()
{

}
