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
#include "QMenu"

class VInfoBox;

class InfoText : public QGraphicsTextItem
{
public:
    InfoText(QString text, VInfoBox *vb);
    virtual ~InfoText();

protected:
    void focusOutEvent(QFocusEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    VInfoBox *vb;
};

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
    QMenu *menu;
    VInfoBox *box;
    bool dragMode;
    QPointF tmpPos;
};

class DInfoBox;

class VInfoBox : public QGraphicsObject
{
    Q_OBJECT
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
    void setInfoText(QString text);

public slots:
    void editText();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    void createMenu();
    QMenu *menu;
    InfoBoxResizeHandle *resizeHandle;
    InfoText *text;
    DInfoBox *data;
    QList<QGraphicsItem *> movingItems;
    QPointF tmpPos;
};

#endif /* end of include guard: INFO_BOX_VEVO6KEU */
