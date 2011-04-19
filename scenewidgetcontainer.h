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


#ifndef SCENEWIDGETCONTAINER_H
#define SCENEWIDGETCONTAINER_H

#include <QGraphicsItem>
#include "QGraphicsProxyWidget"

class SceneWidgetCloseButton : public QGraphicsItem
{
public:
    QGraphicsItem *container;
    SceneWidgetCloseButton();
    void setParent(QGraphicsItem *parent);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class SceneWidgetContainer : public QGraphicsItem
{
public:
    enum {Type = UserType + 3};
    QPoint closepos;
    explicit SceneWidgetContainer();
    ~SceneWidgetContainer();
    void setProxy(QGraphicsProxyWidget *child);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QGraphicsProxyWidget *childproxy;
    int type() const {return Type;}

private:
    SceneWidgetCloseButton close;
};

#endif // SCENEWIDGETCONTAINER_H
