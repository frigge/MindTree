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

#ifndef VIEWPORT

#define VIEWPORT

#include "QDockWidget"
#include "QGLWidget"
#include "QVector3D"
#include "QMatrix4x4"

#include "source/data/nodes/data_node.h"
#include "source/graphics/nodes/graphics_node.h"

class Viewport : public QGLWidget
{
public:
    Viewport(QWidget *parent);

protected:
    void resizeGL(int width, int height);
    void paintGL();
    void initializeGL();
    void drawGrid();
    void drawAxisGizmo();
    void rotateView(qreal xdist, qreal ydist);
    void panView(qreal xdist, qreal ydist);
    void zoomView(qreal xdist, qreal ydist);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QVector3D camlookat;
    QMatrix4x4 transform;
    bool rotate, pan, zoom;
    QPointF lastpos;
};

class ViewportDock;
class QMenu;

class VViewportNode : public VNode
{
public:
    VViewportNode(DNode *node);

protected:
    void contextMenu();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QMenu *cxtM;
};

class ViewportNode : public DNode
{
public:
    ViewportNode(bool raw=false);
    ViewportDock* getDock();

protected:
    virtual VNode* createNodeVis();

private:
    ViewportDock *dock;
    Viewport *view;
};

class ViewportDock : public QDockWidget
{
public:
    ViewportDock(ViewportNode *node);
    Viewport *getViewport();

private:
    Viewport *viewport;
    ViewportNode *node;
};


#endif /* end of include guard: VIEWPORT */
