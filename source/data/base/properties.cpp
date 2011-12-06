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

#include "properties.h"
#include "QTreeWidgetItem"

#include "source/graphics/properties_editor.h"
#include "source/graphics/base/prop_vis.h"
#include "source/data/nodes/data_node_socket.h"

Property::Property(DinSocket *socket)
    : socket(socket), prop_editor(0)
{
}

Property::~Property()
{
    if(prop_editor) {
        delete prop_editor;
        prop_editor = 0;
    }
}

DinSocket* Property::getSocket()    
{
    return socket;
}

QWidget* Property::getEditor()
{
    return prop_editor;
}

void Property::installTreeItem(PropertiesEditor *edit)    
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setFlags(Qt::ItemIsEnabled);
    item->setText(0, getSocket()->getName());

    item->setSizeHint(0, QSize(50, 10));
    item->setSizeHint(1, QSize(50, 10));
    edit->addTopLevelItem(item);
    edit->setItemWidget(item, 1, (QWidget*)getEditor());
}

void Property::setEditor(QWidget* edit)
{
    prop_editor = edit;
}

Property* Property::derived()    
{
    return this;
}

void Property::getValue()    const
{
}

IntProperty::IntProperty(DinSocket *socket)
    : Property(socket), prop(0)
{
}

int IntProperty::getValue()const
{
    return prop;
}

void IntProperty::setValue(int value)
{
    prop = value;
}

QWidget* IntProperty::getEditor()
{
    setEditor(new IntPropWidget(this));
    return Property::getEditor();
}

IntProperty* IntProperty::derived()    
{
    return this;
}

FloatProperty::FloatProperty(DinSocket *socket)
    : Property(socket), prop(0.0f)
{
}

double FloatProperty::getValue()const
{
    return prop;
}

void FloatProperty::setValue(double value)
{
    prop = value;
}

QWidget* FloatProperty::getEditor()
{
    setEditor(new FloatPropWidget(this));
    return Property::getEditor();
}

FloatProperty* FloatProperty::derived()    
{
    return this;
}

VectorProperty::VectorProperty(DinSocket *socket)
    : Property(socket)
{
}

Vector VectorProperty::getValue()const
{
    return prop;
}

void VectorProperty::setValue(Vector value)
{
    prop = value;
}

QWidget* VectorProperty::getEditor()
{
    setEditor(new VectorPropWidget(this));
    return Property::getEditor();
}

VectorProperty* VectorProperty::derived()    
{
    return this;
}

BoolProperty::BoolProperty(DinSocket *socket)
    : Property(socket), prop(false)
{
}

bool BoolProperty::getValue()const
{
    return prop;
}

void BoolProperty::setValue(bool value)
{
    prop = value;
}

QWidget* BoolProperty::getEditor()
{
    QWidget *newEditor = new BoolPropWidget(this);
    setEditor(newEditor);
    return newEditor;
}

BoolProperty* BoolProperty::derived()    
{
    return this;
}

ColorProperty::ColorProperty(DinSocket *socket)
    : Property(socket)
{
}

QColor ColorProperty::getValue()const
{
    return prop;
}

void ColorProperty::setValue(QColor value)
{
    prop = value;
}

QWidget* ColorProperty::getEditor()
{
    setEditor(new ColorPropWidget(this));
    return Property::getEditor();
}

ColorProperty* ColorProperty::derived()    
{
    return this;
}

StringProperty::StringProperty(DinSocket *socket)
    :Property(socket), path(NOPATH)
{
}

void StringProperty::setPath(pathType p)    
{
    path = p;
}

int StringProperty::getPath()    
{
    return path;
}

QString StringProperty::getValue()const
{
    return prop;
}

void StringProperty::setValue(QString value)
{
    prop = value;
}

QWidget* StringProperty::getEditor()
{
    switch(path)
    {
        case NOPATH:
            setEditor(new StringPropWidget(this));
            break;
        case DIRPATH:
        case FILEPATH:
            setEditor(new DirPropWidget(this));
            break;
    }
    return Property::getEditor();
}

StringProperty* StringProperty::derived()    
{
    return this;
}

