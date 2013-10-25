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

#include "data/frg.h"
#include "graphics/windowlist.h"
#include "iostream"
#include "windowfactory.h"
#include "graphics/viewer_dock_base.h"

using namespace MindTree;
WindowFactory::WindowFactory(QString name, std::function<QWidget*()> func)
    : m_action(new QAction(name, 0)), name(name), windowFunc(func)
{
    connect(m_action, SIGNAL(triggered()), this, SLOT(showWindow())); 
}

WindowFactory::WindowFactory(QString name)
    : m_action(new QAction(name, 0)), name(name)
{
    connect(m_action, SIGNAL(triggered()), this, SLOT(showWindow())); 
}

WindowFactory::~WindowFactory()
{
}

QString WindowFactory::getName()    
{
    return name;
}

void WindowFactory::setWindowFunc(std::function<QWidget*()> fn)
{
    windowFunc = fn;
}

QString WindowFactory::showWindow()
{
    auto dock = createWindow();
    MindTree::WindowList::instance()->showDock(dock);
    return dock->objectName();
}

MindTree::ViewerDockBase* WindowFactory::createWindow()    
{
    auto *widget = windowFunc();
    auto *viewer = new MindTree::ViewerDockBase(getName());
    viewer->setWidget(widget);
    return viewer;
}

QAction* WindowFactory::action()    
{
    return m_action;
}

ViewerFactory::ViewerFactory(QString name, QString type, std::function<Viewer*(DoutSocket*)> func)
    : name(name), type(type), windowFunc(func)
{
}

ViewerFactory::ViewerFactory(QString name, QString type)
    : name(name), type(type)
{
}

ViewerFactory::~ViewerFactory()
{
}

QString ViewerFactory::getName()    
{
    return name;
}

QString ViewerFactory::getType()    
{
    return type;
}

void ViewerFactory::setWindowFunc(std::function<Viewer*(DoutSocket*)> fn)
{
    windowFunc = fn;
}

MindTree::ViewerDockBase* ViewerFactory::createViewer(DoutSocket *socket)    
{
    auto *viewer = windowFunc(socket);
    if(!viewer) std::cout<<"could not create the viewer"<<std::endl;
    auto *dock = new MindTree::ViewerDockBase(getName());
    dock->setViewer(viewer);
    return dock;
}
