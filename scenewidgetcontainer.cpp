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


#include "QWidget"
#include "QPainter"
#include "QGraphicsScene"
#include "QGraphicsEffect"

#include "scenewidgetcontainer.h"
#include "shader_space.h"

SceneWidgetCloseButton::SceneWidgetCloseButton()
{
    setZValue(zValue()+0.5);
}

void SceneWidgetCloseButton::setParent(QGraphicsItem *parent)
{
    container = parent;
    setParentItem(parent);
}

QRectF SceneWidgetCloseButton::boundingRect() const
{
    return QRectF(0, 0, 30, 30);
}

void SceneWidgetCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(180, 180, 180, 100)));
    painter->drawEllipse(0, 0, 30, 30);

    QPen cross;
    cross.setWidth(5);
    cross.setColor(QColor(0, 0, 0));
    painter->setPen(cross);

    painter->drawLine(10, 10, 20, 20);
    painter->drawLine(20, 10, 10, 20);
};

void SceneWidgetCloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    SceneWidgetContainer *cont = dynamic_cast<SceneWidgetContainer *>(container);
    Shader_Space *space = dynamic_cast<Shader_Space *>(cont->scene());
    space->destroyContainer(cont);
}

SceneWidgetContainer::SceneWidgetContainer()
{

}

SceneWidgetContainer::~SceneWidgetContainer()
{
}

void SceneWidgetContainer::setProxy(QGraphicsProxyWidget *child)
{
    childproxy = child;
    QWidget *childwidget = child->widget();
    child->setParentItem(this);
    closepos = QPoint(childwidget->width(), -30);
    close.setParent(this);
    close.setPos((QPointF)closepos);

    setZValue(zValue()+0.5);
    setFlag(ItemIsMovable, true);
    QGraphicsDropShadowEffect drop;
    drop.setBlurRadius(10);
    drop.setOffset(5);
    drop.setColor(QColor(10, 10, 10, 200));
    setGraphicsEffect(&drop);
}

QRectF SceneWidgetContainer::boundingRect() const
{
    if(!childproxy)
        return QRectF();
    QWidget *childwidget = childproxy->widget();
    return QRectF(-10, -50, childwidget->width()+20, childwidget->height()+60);
}

void SceneWidgetContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QWidget *childwidget = childproxy->widget();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(120, 120, 160, 100));
    QPainterPath path;
    path.addRoundedRect(-10, -10, childwidget->width()+20, childwidget->height()+20, 5, 5);
    path.addEllipse(closepos + QPoint(15, 15), 25, 25);
    path.setFillRule(Qt::WindingFill);
    painter->drawPath(path);
//    painter->drawRoundedRect(-10, -10, childwidget->width()+20, childwidget->height()+20, 5, 5);

//    painter->drawEllipse(closepos + QPoint(15, 15), 25, 25);
//    childwidget->update();
}
