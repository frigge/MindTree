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

#include "prop_vis.h"

#include "QVBoxLayout"
#include "QColorDialog"

#include "source/data/base/frg.h"
#include "source/data/nodes/data_node_socket.h"

StringPropWidget::StringPropWidget(StringProperty *prop)
    : prop(prop)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(setProp(QString)));
    connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
    setText(prop->getValue());
}

StringPropWidget::~StringPropWidget()
{
    prop->setEditor(0);
}

QSize StringPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void StringPropWidget::setProp(QString text)    
{
    ((StringProperty*)prop)->setValue(text);
    emit change(prop->getSocket()->getNode()); 
}

IntPropWidget::IntPropWidget(IntProperty *prop)
    : prop(prop)
{
    setRange(-1000000, 1000000);
    setMaximum(1000000);
    setMinimum(-1000000);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(setProp(int)));
    connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
    setValue(prop->getValue());
}

QSize IntPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

IntPropWidget::~IntPropWidget()
{
    prop->setEditor(0);
}

void IntPropWidget::setProp(int val)    
{
    ((IntProperty*)prop)->setValue(val); 
    emit change(prop->getSocket()->getNode()); 
}

BoolPropWidget::BoolPropWidget(BoolProperty *prop)
    : prop(prop)
{
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(setProp(int)));
    connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
    setChecked(prop->getValue());
}

BoolPropWidget::~BoolPropWidget()
{
    prop->setEditor(0);
}

QSize BoolPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void BoolPropWidget::setProp(int checked)    
{
    ((BoolProperty*)prop)->setValue((bool)checked);
    emit change(prop->getSocket()->getNode()); 
}

FloatPropWidget::FloatPropWidget(FloatProperty *prop)
    : prop(prop)
{
    setRange(-1000000, 1000000);
    setDecimals(3);
    setSingleStep(0.1);

    connect(this, SIGNAL(valueChanged(double)), this, SLOT(setProp(double)));
    connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));

    setValue(prop->getValue());
}

FloatPropWidget::~FloatPropWidget()
{
    prop->setEditor(0);
}

QSize FloatPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void FloatPropWidget::setProp(double val)    
{
    ((FloatProperty*)prop)->setValue(val); 
    emit change(prop->getSocket()->getNode()); 
}

VectorValue::VectorValue()
{
    setContentsMargins(0, 0, 0, 0);
    lay = new QVBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    spinx = new QDoubleSpinBox();
    spiny = new QDoubleSpinBox();
    spinz = new QDoubleSpinBox();
    spinx->setRange(-1000000, 1000000);
    spiny->setRange(-1000000, 1000000);
    spinz->setRange(-1000000, 1000000);
    lay->setMargin(0);
    lay->addWidget(spinx);
    lay->addWidget(spiny);
    lay->addWidget(spinz);
    setLayout(lay);
    connect(spinx, SIGNAL(valueChanged(double)), this, SLOT(emitValueChanged()));
    connect(spiny, SIGNAL(valueChanged(double)), this, SLOT(emitValueChanged()));
    connect(spinz, SIGNAL(valueChanged(double)), this, SLOT(emitValueChanged()));
}

void VectorValue::emitValueChanged()    
{
    Vector value(spinx->value(), spiny->value(), spinz->value());
    emit valueChanged(value); 
}

void VectorValue::setValue(Vector value)    
{
    spinx->setValue(value.x); 
    spiny->setValue(value.y); 
    spinz->setValue(value.z); 
}

VectorPropWidget::VectorPropWidget(VectorProperty *prop)
    : prop(prop)
{
    connect(this, SIGNAL(valueChanged(Vector)), this, SLOT(setProp(Vector)));
    connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
    setValue(prop->getValue());
}

VectorPropWidget::~VectorPropWidget()
{
    prop->setEditor(0);
}

QSize VectorPropWidget::sizeHint()    const
{
    return QSize(10, 54);
}

void VectorPropWidget::setProp(Vector vec)    
{
    ((VectorProperty*)prop)->setValue(vec);
    emit change(prop->getSocket()->getNode()); 
}

ColorPropWidget::ColorPropWidget(ColorProperty *prop)
    : prop(prop)
{
    setAutoFillBackground(true);
    setPalette(QPalette(prop->getValue()));
    setFlat(true);
    connect(this, SIGNAL(clicked()), this, SLOT(setColor()));

    connect(this, SIGNAL(clicked(QColor)), this, SLOT(setProp(QColor)));
    connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
    setPalette(QPalette(prop->getValue()));
}

ColorPropWidget::~ColorPropWidget()
{
    prop->setEditor(0);
}

QSize ColorPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void ColorPropWidget::setProp(QColor col)    
{
    ((ColorProperty*)prop)->setValue(col);
    emit change(prop->getSocket()->getNode()); 
}

void ColorPropWidget::setColor()
{
    QColor newcolor = QColorDialog::getColor(((ColorProperty*)prop)->getValue());
    if(!newcolor.isValid()) return;
    setPalette(QPalette(newcolor));
    emit clicked(newcolor);
    update();
}

