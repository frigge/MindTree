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


#ifndef SHADER_VIEW_H
#define SHADER_VIEW_H

#include "QGraphicsView"
#include "QGraphicsScene"
#include "QMainWindow"
#include "QDockWidget"
#include "QWidget"
#include "QAction"
#include "QGraphicsRectItem"

namespace MindTree
{
class DNSpace;
} /* MindTree */
    
class ChangeSpaceAction : public QAction
{
    Q_OBJECT
public:
    ChangeSpaceAction(MindTree::DNSpace *space, QObject *parent);

signals:
    void triggered(MindTree::DNSpace *space, bool checked = false);

private slots:
    void emitTriggered();

private:
    MindTree::DNSpace *new_space;
};

class VNSpace;
class Shader_View;
class NodeGraphWidget : public QWidget
{
    Q_OBJECT
public:
    NodeGraphWidget();
    virtual ~NodeGraphWidget();
    void updateToolBar();

public slots:
    void setRoot_Space();
    void moveIntoSpace(MindTree::DNSpace *space);

private:
    Shader_View *view;
    QList<MindTree::DNSpace*>spaces;
    QList<ChangeSpaceAction*>spaceActions;
    QToolBar *toolbar;
};

class NodeGraphDock : public QDockWidget
{
public:
    NodeGraphDock();
    virtual ~NodeGraphDock();

private:
};

class Shader_View : public QGraphicsView
{
public:
    Shader_View();
    void createSpace();
    VNSpace *getSpace();
    void adjustViewRPrev();
    void adjustSceneRect(QPoint mousePos);
    void testWidgets();
    void adjustResizeItems();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void updateSceneRect();
    void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);
    void wheelEvent(QWheelEvent *event);
    //void keyPressEvent(QKeyEvent *event);
    //void keyReleaseEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    QRectF visibleRect();

private:
    bool drag;
    QGraphicsRectItem *spaceR, *viewR, *bottomWidget, *topLeft, *bottomRight;
    QPoint lastPos;
};

#endif // SHADER_VIEW_H
