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

#include "iostream"

#include "graphics/windowlist.h"
#ifndef Q_MOC_RUN
#include "graphics/viewer_dock_base.h"
#endif

#include "windowfactory.h"
#include "../graphics/viewer.h"

using namespace MindTree;

AbstractGuiFactory::AbstractGuiFactory(QString name)
    : _name(name)
{

}

AbstractGuiFactory::~AbstractGuiFactory()
{
}

QString AbstractGuiFactory::getName() const
{
    return _name;
}

WindowFactory::WindowFactory(QString name, std::function<QWidget*()> func)
    : AbstractGuiFactory(name), m_action(new QAction(name, 0)), windowFunc(func)
{
    connect(m_action, SIGNAL(triggered()), this, SLOT(showWindow())); 
}

WindowFactory::WindowFactory(QString name)
    : AbstractGuiFactory(name), m_action(new QAction(name, 0))
{
    connect(m_action, SIGNAL(triggered()), this, SLOT(showWindow())); 
}

WindowFactory::~WindowFactory()
{
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
    auto *viewer = new MindTree::ViewerDockBase(getName(), this);
    viewer->setWidget(widget);
    return viewer;
}

QAction* WindowFactory::action()    
{
    return m_action;
}

ViewerFactory::ViewerFactory(QString name, QString type, std::function<Viewer*(DoutSocket*)> func)
    : AbstractGuiFactory(name), type(type), windowFunc(func), _dock(nullptr)
{
}

ViewerFactory::ViewerFactory(QString name, QString type)
    : AbstractGuiFactory(name), type(type), _dock(nullptr)
{
}

ViewerFactory::~ViewerFactory()
{
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
    viewer->initBase();
    if(!viewer) std::cout<<"could not create the viewer"<<std::endl;
    _dock = new MindTree::ViewerDockBase(getName(), this);
    _dock->setViewer(viewer);
    return _dock;
}

void ViewerFactory::setActive(ViewerDockBase *dock)
{
    _dock = dock;
}

ViewerDockBase* ViewerFactory::getActive() const
{
    return _dock;
}


MindTree::ViewerDockBase* ViewerFactory::getViewer(DoutSocket *socket)
{
    if(!_dock)
        return createViewer(socket);

    return _dock;
}
