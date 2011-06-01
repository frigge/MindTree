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

#ifndef GRAPHICS_NODE_H
#define GRAPHICS_NODE_H

#include "QHash"
#include "QPushButton"
#include "QCheckBox"
#include "QGridLayout"
#include "QGraphicsTextItem"
#include "QGraphicsView"
#include "QGraphicsItem"
#include "QObject"

#include "source/data/nodes/data_node.h"
#include "source/graphics/nodes/graphics_node_socket.h"

class NodeName : public QGraphicsTextItem
{
public:
    NodeName(QString name, QGraphicsItem *parent);

protected:
    void focusOutEvent(QFocusEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

class VNode : public QGraphicsItem
{
public:
    VNode(DNode *data);
    DNode *data;
    enum {Type = UserType + 2};
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * = 0);
    int type() const {return Type;}
    NodeName *node_name;
    void drawName();

protected:
    virtual void init();
    virtual int NodeWidth() const;
    virtual int NodeHeight(int numSockets) const;
};

class ContainerNode;

class VContainerNode : public QObject, public VNode
{
    Q_OBJECT
public:
    VContainerNode(DNode*);

public slots:
    void addToLib();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    void createContextMenu();
    QMenu *contextMenu;
};

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    ColorButton();

public slots:
    void setColor();

signals:
    void clicked(QColor);
};

class VValueNode : public QObject, public VNode
{
    Q_OBJECT
public:
    VValueNode(DNode *);
    void setValueEditor(QWidget *editor);

public slots:
    void setShaderInput(bool isInput);

protected:
    QGraphicsProxyWidget *proxy;
    virtual int NodeWidth() const;
    virtual int NodeHeight(int numSockets) const;
    QWidget *widget;
    QWidget *base_widget;
    QCheckBox *shader_parameter;
    QGridLayout *lay;
    QMenu *contextMenu;
    void createContextMenu();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void init();
};

class VColorValueNode : public VValueNode
{
    Q_OBJECT
public:
    VColorValueNode(DNode *);

public slots:
    void setValue(QColor);

protected:
    virtual void init();
};

class VStringValueNode : public VValueNode
{
    Q_OBJECT
public:
    VStringValueNode(DNode *data);

public slots:
    void setValue(QString);

protected:
    virtual void init();
};

class VFloatValueNode : public VValueNode
{
    Q_OBJECT
public:
    VFloatValueNode(DNode *data);

public slots:
    void setValue(double);

protected:
    virtual void init();
};

class VVectorValueNode : public VValueNode
{
    Q_OBJECT
public:
    VVectorValueNode(DNode *data);

public slots:
    void setValue();

protected:
    virtual void init();
};

class VOutputNode : public QObject, public VNode
{
    Q_OBJECT
public:
    VOutputNode(DNode *data);

public slots:
    void writeCode();
    void changeName();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void init();

private:
    void createMenu();
    QMenu *contextMenu;
};


#endif // GRAPHICS_NODE_H
