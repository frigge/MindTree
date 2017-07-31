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

#include "graphics/viewport_widget.h"
#include "graphics/windowlist.h"
#include "data/windowfactory.h"
#include "boost/python.hpp"
#include "pluginentry.h"

#include <QCoreApplication>

namespace BPy = boost::python;

using namespace MindTree;

MindTree::Viewer* addViewport(MindTree::DoutSocket *socket)    
{
    return new ViewportViewer(socket);
}

BOOST_PYTHON_MODULE(scenegraph){
    ViewerList::instance()
        ->addViewer(new MindTree::ViewerFactory("&Viewport", 
                                                "GROUPDATA", 
                                                addViewport));

    ViewerList::instance()
        ->addViewer(new MindTree::ViewerFactory("&Viewport", 
                                                "TRANSFORMABLE", 
                                                addViewport));
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
}
