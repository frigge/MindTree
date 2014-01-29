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

#ifndef PROP_VIS_O8D0XSQG

#define PROP_VIS_O8D0XSQG

#include "QLineEdit"
#include "QSpinBox"
#include "QDoubleSpinBox"
#include "QCheckBox"
#include "QPushButton"
#include "QHBoxLayout"
#include "data/datatypes.h"

#include "data/properties.h"

class QVBoxLayout;
namespace MindTree
{
class DNode;

class StringPropWidget : public QLineEdit
{
    Q_OBJECT
public:
    StringPropWidget();
    QSize sizeHint() const;
    ~StringPropWidget();

public slots:
    void setProp(QString text);

signals:
    void change(DNode*);

private:
};

class DirPropWidget : public QWidget
{
    Q_OBJECT
public:
    DirPropWidget();
    virtual ~DirPropWidget();
    QSize sizeHint() const;

public slots:
    void browse();

private:
    StringPropWidget *textBox;
    QPushButton *dirButton;
    QHBoxLayout lay;
};

class IntPropWidget : public QSpinBox
{
    Q_OBJECT
public:
    IntPropWidget();
    ~IntPropWidget();
    QSize sizeHint() const;

public slots:
    void setProp(int val);

signals:
    void change(DNode*);

private:
};

class BoolPropWidget : public QCheckBox
{
    Q_OBJECT
public:
    BoolPropWidget();
    ~BoolPropWidget();
    QSize sizeHint() const;

public slots:
    void setProp(int checked);

signals:
    void change(DNode*);

private:
};

class FloatPropWidget : public QDoubleSpinBox
{
    Q_OBJECT
public:
    FloatPropWidget();
    ~FloatPropWidget();
    QSize sizeHint() const;

public slots:
    void setProp(double val);

signals:
    void change(DNode*);

private:
};

class VectorValue : public QWidget
{
    Q_OBJECT
public:
    VectorValue();
    void setValue(Vector value);

public slots:
    void emitValueChanged();

signals:
    void valueChanged(Vector);

private:
    QDoubleSpinBox *spinx, *spiny, *spinz;
    QVBoxLayout *lay;
};

class VectorPropWidget : public VectorValue
{
    Q_OBJECT
public:
    VectorPropWidget();
    ~VectorPropWidget();
    QSize sizeHint() const;

public slots:
    void setProp(Vector vec);

signals:
    void change(DNode*);

private:
};

class ColorPropWidget : public QPushButton
{
    Q_OBJECT
public:
    ColorPropWidget();
    ~ColorPropWidget();
    QSize sizeHint() const;

public slots:
    void setColor();
    void setProp(QColor col);

signals:
    void clicked(QColor);
    void change(DNode*);


private:
};

    
} /* MindTree */
#endif /* end of include guard: PROP_VIS_O8D0XSQG */
