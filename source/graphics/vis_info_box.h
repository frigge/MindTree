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

#ifndef INFO_BOX_VEVO6KEU

#define INFO_BOX_VEVO6KEU

#include "QGraphicsItem"

class VInfoBox;

class InfoBoxResizeHandle : public QGraphicsItem
{
public:
    InfoBoxResizeHandle(VInfoBox *box);
    virtual ~InfoBoxResizeHandle();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    VInfoBox *box;
    bool dragMode;
    QPointF tmpPos;
};

class DInfoBox;

class VInfoBox : public QGraphicsItem
{
public:
    VInfoBox(DInfoBox* data);
    virtual ~VInfoBox();

    enum {Type = UserType + 3};
    int type() const {return Type;}

    DInfoBox* getData();
    void setData(DInfoBox* value);
    int getWidth() const;
    int getHeight() const;
    void setWidth(int w);
    void setHeight(int h);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    InfoBoxResizeHandle *resizeHandle;
    QGraphicsTextItem *name, *text;
    DInfoBox *data;
};

#endif /* end of include guard: INFO_BOX_VEVO6KEU */
