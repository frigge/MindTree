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

#include "data/frg.h"
#include "source/graphics/base/vnspace.h"

using namespace MindTree;

PropertiesEditorItem::PropertiesEditorItem(SocketProperty *prop, PropertiesEditor *edit)
    : prop(prop), edit(edit)
{
    setFlags(Qt::ItemIsEnabled);
    QWidget *itemWidget = new QWidget();
    QHBoxLayout *lay = new QHBoxLayout();
    itemWidget->setLayout(lay);
    lay->addWidget(new QLabel(prop->getSocket()->getName()));
    lay->addWidget((QWidget*)prop->getEditor());
    lay->setMargin(0);
    lay->setSpacing(0);

    setSizeHint(0, QSize(50, 10));
    edit->addTopLevelItem(this);
    edit->setItemWidget(this, 0, itemWidget);
}

PropertiesEditorItem::~PropertiesEditorItem()
{
}

PropertiesEditor::PropertiesEditor(QWidget *parent)
    : QTreeWidget(parent)
{
    FRG::propEditor = this;
    setColumnCount(1);
    setAlternatingRowColors(true);
    setHeaderHidden(true);
    //connect(FRG::Space, SIGNAL(selectionChanged()), this, SLOT(updateFocus()));
}

QSize PropertiesEditor::sizeHint()    const
{
    return QSize(400, 400); 
}

DNode* PropertiesEditor::getFocusNode()
{
    return focus_node;
}

void PropertiesEditor::updateFocus()    
{
    //if(FRG::Space->selectedNodes().isEmpty())
        //clear();
    //else
    //    setFocusNode(FRG::Space->selectedNodes().first()); 
}

void PropertiesEditor::setFocusNode(DNode* value)
{
    focus_node = value;
    
    clear();

    //LLsocket *tmp = value->getInSocketLlist()->getFirst();
    //if(!tmp)return;
    //do{
    //    if(!tmp->socket->getArray() 
    //        && !tmp->socket->toIn()->getCntdSocket())
    //        //new PropertiesEditorItem(tmp->socket->toIn()->getSocketProperty(), this);
    //        //tmp->socket->toIn()->getSocketProperty()->installTreeItem(this);
    //}while(tmp = tmp->next);
}
