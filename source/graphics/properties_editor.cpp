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

#include "properties_editor.h"

#include "QDoubleSpinBox"
#include "QSpinBox"
#include "QLineEdit"
#include "QLabel"
#include "QCheckBox"

#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"

PropertiesEditor::PropertiesEditor(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setAlternatingRowColors(true);
    setHeaderHidden(true);
    connect(FRG::Space, SIGNAL(selectionChanged()), this, SLOT(updateFocus()));
}

DNode* PropertiesEditor::getFocusNode()
{
    return focus_node;
}

void PropertiesEditor::updateFocus()    
{
    if(FRG::Space->selectedNodes().isEmpty())
        clear();
    else
        setFocusNode(FRG::Space->selectedNodes().first()); 
}

void PropertiesEditor::setFocusNode(DNode* value)
{
    focus_node = value;
    
    clear();

    foreach(DinSocket *socket, focus_node->getInSockets()){
        Property *prop = socket->getProperty();
        if(prop && !socket->getCntdSocket())
            prop->installTreeItem(this);
    }
}
