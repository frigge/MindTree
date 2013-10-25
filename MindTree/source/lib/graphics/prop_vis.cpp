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
#include "QFileDialog"

#include "data/frg.h"
#include "data/nodes/data_node_socket.h"

using namespace MindTree;

StringPropWidget::StringPropWidget()
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(setProp(QString)));
    //connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
}

StringPropWidget::~StringPropWidget()
{
}

QSize StringPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void StringPropWidget::setProp(QString text)    
{
}

DirPropWidget::DirPropWidget()
    : textBox(new StringPropWidget()), dirButton(new QPushButton("..."))
{
    setLayout(&lay);
    lay.setMargin(0);
    lay.setSpacing(0);
    dirButton->resize(10, 18);
    connect(dirButton, SIGNAL(clicked()), this, SLOT(browse()));
    lay.addWidget(textBox);
    lay.addWidget(dirButton);
}

DirPropWidget::~DirPropWidget()
{
    delete textBox;
    delete dirButton; 
}

QSize DirPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void DirPropWidget::browse()    
{
    QString newdir;
    QStringList newdirs;
    bool ok;

    textBox->setText(newdir);
}

IntPropWidget::IntPropWidget()
{
    setRange(-1000000, 1000000);
    setMaximum(1000000);
    setMinimum(-1000000);
    //connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
}

QSize IntPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

IntPropWidget::~IntPropWidget()
{
}

void IntPropWidget::setProp(int val)    
{
}

BoolPropWidget::BoolPropWidget()
{
    //connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
}

BoolPropWidget::~BoolPropWidget()
{
}

QSize BoolPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void BoolPropWidget::setProp(int checked)    
{
}

FloatPropWidget::FloatPropWidget()
{
    setRange(-1000000, 1000000);
    setDecimals(3);
    setSingleStep(0.1);
    //connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
}

FloatPropWidget::~FloatPropWidget()
{
}

QSize FloatPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void FloatPropWidget::setProp(double val)    
{
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

VectorPropWidget::VectorPropWidget()
{
    connect(this, SIGNAL(valueChanged(Vector)), this, SLOT(setProp(Vector)));
    //connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
}

VectorPropWidget::~VectorPropWidget()
{
}

QSize VectorPropWidget::sizeHint()    const
{
    return QSize(10, 54);
}

void VectorPropWidget::setProp(Vector vec)    
{
}

ColorPropWidget::ColorPropWidget()
{
    setAutoFillBackground(true);
    setFlat(true);
    connect(this, SIGNAL(clicked()), this, SLOT(setColor()));

    connect(this, SIGNAL(clicked(QColor)), this, SLOT(setProp(QColor)));
    //connect(this, SIGNAL(change(DNode*)), (QObject*)FRG::Space, SIGNAL(linkChanged(DNode*)));
}

ColorPropWidget::~ColorPropWidget()
{
}

QSize ColorPropWidget::sizeHint()    const
{
    return QSize(10, 18);
}

void ColorPropWidget::setProp(QColor col)    
{
}

void ColorPropWidget::setColor()
{
    update();
}

