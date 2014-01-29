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

#include "cmath"
#include "QTreeWidgetItem"
#include "QLabel"
#include "QHBoxLayout"

#include "data_node_socket.h"
#include "source/graphics/properties_editor.h"
#include "graphics/prop_vis.h"

#include "socket_properties.h"

using namespace MindTree;

SocketProperty::SocketProperty(MindTree::DinSocket *socket)
    : socket(socket), prop_editor(0)
{
}

SocketProperty::~SocketProperty()
{
    if(prop_editor) {
        delete prop_editor;
        prop_editor = 0;
    }
}

MindTree::DinSocket* SocketProperty::getSocket()    
{
    return socket;
}

QWidget* SocketProperty::getEditor()
{
    return prop_editor;
}

void SocketProperty::installTreeItem(PropertiesEditor *edit)    
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setFlags(Qt::ItemIsEnabled);
    QWidget *itemWidget = new QWidget();
    QHBoxLayout *lay = new QHBoxLayout();
    itemWidget->setLayout(lay);
    lay->addWidget(new QLabel(getSocket()->getName().c_str()));
    lay->addWidget((QWidget*)getEditor());
    lay->setMargin(0);
    lay->setSpacing(0);

    item->setSizeHint(0, QSize(50, 10));
    edit->addTopLevelItem(item);
    edit->setItemWidget(item, 0, itemWidget);
}

void SocketProperty::setEditor(QWidget* edit)
{
    prop_editor = edit;
}

SocketProperty* SocketProperty::derived()    
{
    return this;
}

void SocketProperty::getValue()    const
{
}

IntSocketProperty::IntSocketProperty(MindTree::DinSocket *socket)
    : SocketProperty(socket), prop(0)
{
}

int IntSocketProperty::getValue()const
{
    return prop;
}

void IntSocketProperty::setValue(int value)
{
    prop = value;
}

QWidget* IntSocketProperty::getEditor()
{
    setEditor(new IntPropWidget());
    return SocketProperty::getEditor();
}

IntSocketProperty* IntSocketProperty::derived()    
{
    return this;
}

FloatSocketProperty::FloatSocketProperty(MindTree::DinSocket *socket)
    : SocketProperty(socket), prop(0.0f)
{
}

double FloatSocketProperty::getValue()const
{
    return prop;
}

void FloatSocketProperty::setValue(double value)
{
    prop = value;
}

QWidget* FloatSocketProperty::getEditor()
{
    setEditor(new FloatPropWidget());
    return SocketProperty::getEditor();
}

FloatSocketProperty* FloatSocketProperty::derived()    
{
    return this;
}

VectorSocketProperty::VectorSocketProperty(MindTree::DinSocket *socket)
    : SocketProperty(socket)
{
}

Vec3d VectorSocketProperty::getValue()const
{
    return prop;
}

void VectorSocketProperty::setValue(Vec3d value)
{
    prop = value;
}

QWidget* VectorSocketProperty::getEditor()
{
    setEditor(new VectorPropWidget());
    return SocketProperty::getEditor();
}

VectorSocketProperty* VectorSocketProperty::derived()    
{
    return this;
}

BoolSocketProperty::BoolSocketProperty(MindTree::DinSocket *socket)
    : SocketProperty(socket), prop(false)
{
}

bool BoolSocketProperty::getValue()const
{
    return prop;
}

void BoolSocketProperty::setValue(bool value)
{
    prop = value;
}

QWidget* BoolSocketProperty::getEditor()
{
    QWidget *newEditor = new BoolPropWidget();
    setEditor(newEditor);
    return newEditor;
}

BoolSocketProperty* BoolSocketProperty::derived()    
{
    return this;
}

ColorSocketProperty::ColorSocketProperty(MindTree::DinSocket *socket)
    : SocketProperty(socket)
{
}

QColor ColorSocketProperty::getValue()const
{
    return prop;
}

void ColorSocketProperty::setValue(QColor value)
{
    prop = value;
}

QWidget* ColorSocketProperty::getEditor()
{
    setEditor(new ColorPropWidget());
    return SocketProperty::getEditor();
}

ColorSocketProperty* ColorSocketProperty::derived()    
{
    return this;
}

StringSocketProperty::StringSocketProperty(MindTree::DinSocket *socket)
    :SocketProperty(socket), path(NOPATH)
{
}

void StringSocketProperty::setPath(pathType p)    
{
    path = p;
}

int StringSocketProperty::getPath()    
{
    return path;
}

QString StringSocketProperty::getValue()const
{
    return prop;
}

void StringSocketProperty::setValue(QString value)
{
    prop = value;
}

QWidget* StringSocketProperty::getEditor()
{
    switch(path)
    {
        case NOPATH:
            setEditor(new StringPropWidget());
            break;
        case DIRPATH:
        case FILEPATH:
            setEditor(new DirPropWidget());
            break;
    }
    return SocketProperty::getEditor();
}

StringSocketProperty* StringSocketProperty::derived()    
{
    return this;
}


