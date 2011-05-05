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


#include "QtGui"

#include "QGraphicsItem"
#include "QGraphicsView"
#include "QGraphicsScene"
#include "QPainterPath"
#include "QWheelEvent"
#include "QPalette"
#include "QMenu"

#include "math.h"

#include "frg_shader_author.h"
#include "shader_space.h"
#include "node.h"
#include "shader_view.h"

ChangeSpaceAction::ChangeSpaceAction(QGraphicsScene *space, QObject *parent)
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
    filename = "";
    setWindowTitle("frg Shader Author");
    resize(800, 600);

    root_scene = new Shader_Space();
    Shader_Space *root_space = (Shader_Space*)root_scene;
    root_space->setName("Root");

    QHBoxLayout *layout = new QHBoxLayout;

    view = new Shader_View(this);

    view->setScene(root_space);
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setUpdatesEnabled(true);

    toolbar = addToolBar("");
    updateToolBar();

    layout->addWidget(view);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    createMenus();

    statusBar()->showMessage("Welcome to frg Shader Author");
    setCentralWidget(widget);

};

void frg_Shader_Author::moveIntoSpace(QGraphicsScene *space)
{
    view->setScene(space);
    spaces.append(space);
    updateToolBar();
}

void frg_Shader_Author::updateToolBar()
{
    foreach(QAction *action, toolbar->actions())
        toolbar->removeAction(action);

    QAction *rootAction = toolbar->addAction("Root");
    connect(rootAction, SIGNAL(triggered()), this, SLOT(setRoot_Space()));

    Shader_Space *space = dynamic_cast<Shader_Space *>(view->scene());

    ChangeSpaceAction *chspace;
    while(!spaceActions.isEmpty())
        delete spaceActions.takeLast();

    foreach(QGraphicsScene *scene, spaces)
    {
        Shader_Space *space =(Shader_Space*)scene;
        ChangeSpaceAction *chspace = new ChangeSpaceAction(space, toolbar);
        spaceActions.append(chspace);
        chspace->setText(space->name);
        toolbar->addAction(chspace);
        connect(chspace, SIGNAL(triggered(QGraphicsScene*)), this, SLOT(setShader_Space(QGraphicsScene*)));
    }
}

void frg_Shader_Author::setShader_Space(QGraphicsScene *space)
{
    view->setScene(space);
    if(spaces.endsWith(space))
        return;
    QGraphicsScene *tmp = spaces.takeLast();
    while(tmp != space)
        tmp = spaces.takeLast();
    spaces.append(tmp);
    updateToolBar();
}

void frg_Shader_Author::setRoot_Space()
{
    view->setScene(root_scene);
    spaces.clear();
    updateToolBar();
}

void frg_Shader_Author::createMenus()
{
    QMenu *fileMenu = new QMenu("&File");
    QAction *newAction = fileMenu->addAction("&New");
    QAction *openAction = fileMenu->addAction("&Open");
    QAction *saveAction = fileMenu->addAction("&Save");
    QAction *SaveAsAction = fileMenu->addAction("Save &As");
    QAction *quitAction = fileMenu->addAction("&Quit");

    connect(newAction, SIGNAL(triggered()), this, SLOT(newfile()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    connect(SaveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    menuBar()->addMenu(fileMenu);
};

void frg_Shader_Author::change_window_title(QString title)
{
    QString newtitle("frg Shader Author");
    newtitle.append(" - ");
    newtitle.append(title);
    setWindowTitle(newtitle);
}

void frg_Shader_Author::newfile()
{
    Shader_Space *space = new Shader_Space;
    view->setScene(space);
    root_scene = space;
    foreach(QGraphicsScene *scene, spaces)
        delete scene;
    spaces.clear();
    change_window_title("");
}

void frg_Shader_Author::save()
{
    if(filename == "")
    {
        filename = QFileDialog::getSaveFileName(this);
    }
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    Shader_Space *space = (Shader_Space*)root_scene;
    stream<<space;
    file.close();
    change_window_title(filename);
}

void frg_Shader_Author::saveAs()
{
    filename = QFileDialog::getSaveFileName(this);
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    Shader_Space *space = (Shader_Space*)root_scene;
    stream<<space;
    file.close();
    change_window_title(filename);
}

void frg_Shader_Author::open()
{
    filename = QFileDialog::getOpenFileName(this);
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    Shader_Space *space;
    stream>>&space;
    view->setScene(space);
    root_scene = space;
    file.close();
    change_window_title(filename);
}


frg_Shader_Author::~frg_Shader_Author()
{

}
