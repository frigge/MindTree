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

#include "QTextStream"
#include "QFile"

#include "data/nodes/data_node.h"
#include "data/frg.h"
#include "source/graphics/base/mindtree_mainwindow.h"
#include "source/data/code_generator/outputs.h"

using namespace MindTree;

SourceEdit::SourceEdit(QWidget *parent)
    : QTextEdit(parent)
{
}

void SourceEdit::load()    
{
}

void SourceEdit::load(DNode *node)    
{
    //if(!node->getNodeType() == TEXTVIEWER)
    //    return;

    //TextViewerNode *tnode = node->getDerived<TextViewerNode>();
    //document()->setPlainText();
}

SourceDock::SourceDock()
    : ViewerDockBase("Source"), edit(new SourceEdit(this))
{
    setWidget(edit);
    //FRG::Author->addDockWidget(Qt::RightDockWidgetArea, this);
   // connect((QObject*)FRG::Space, SIGNAL(linkChanged()), (QObject*)edit, SLOT(load()));
   // connect((QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)), (QObject*)edit, SLOT(load(DNode*)));
    connect((QObject*)this, SIGNAL(visibilityChanged(bool)), (QObject*)edit, SLOT(load()));
    hide();
}

SourceDock::~SourceDock()
{
    //FRG::Author->removeDockWidget(this);
    delete edit;
}

//void SourceDock::changeViewerNode(ViewerNodeBase* node)    
//{
//    if(node->getNodeType() == TEXTVIEWER)
//        setNode(node);
//}

//void SourceDock::setFocusNode(ViewerNodeBase *node)    
//{
//    ViewerDockBase::setFocusNode(node);
//}

void SourceDock::exec()    
{
}

void SourceDock::exec(DNode* node)    
{
}
