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

#include "source/graphics/sourcedock.h"

SourceEdit::SourceEdit(QWidget *parent, OutputNode *node)
    : QTextEdit(parent), outnode(node)
{
}

void SourceEdit::load(DNode *node)    
{
    if(node && outnode.getAllInNodes().contains(node))
     {

     }
}

SourceDock::SourceDock(OutputNode *node)
    : QDockWidget("Source: "+ node->getNodeName()), edit(new SourceEdit(this, node)) 
{
    setWidget(edit);
    FRG::Author->addDockWidget(Qt::RightDockWidgetArea, this);
    connect((QObject*)FRG::Space, SIGNAL(linkChanged()), (QObject*)edit, SLOT(load()));
    connect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), (QObject*)edit, SLOT(load(DNode*)));
    hide();
}

SourceDock::~SourceDock()
{
    FRG::Author->removeDockWidget(this);
    delete edit;
}
