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
#include "QTimer"
#include "QThread"

#include "source/data/nodes/data_node.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/data/scene/object.h"

class SceneCache;
class ViewportNode;

class Viewport : public QGLWidget
{
    Q_OBJECT
public:
    Viewport(QWidget *parent, ViewportNode *node);

public slots:
    void render();
    void render(DNode *node);
    void repaint();

protected:
    void resizeGL(int width, int height);
    void paintGL();
    void initializeGL();
    void drawGrid();
    void drawScene();
    void drawObject(Object* obj);
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
    SceneCache *cache;
    ViewportNode *viewnode;
    QTimer timer;
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

class ViewportNode;
class CacheThread : public QThread
{
public:
    CacheThread();
    void run();
    void setNode(DNode *node);
    void setCache(SceneCache **cache);
    void setStart(ViewportNode *viewnode);
    void setViewport(Viewport *view);

private:
    SceneCache **cache;
    ViewportNode *view;
    Viewport* viewport;
    DNode *node;
};

class ViewportNode : public DNode
{
public:
    ViewportNode(bool raw=false);
    ViewportDock* getDock();
    void render(SceneCache **cache, DNode *node=0);

protected:
    virtual VNode* createNodeVis();

private:
    ViewportDock *dock;
    Viewport *view;
    CacheThread *thread;
};

class ViewportDock : public QDockWidget
{
    Q_OBJECT
public:
    ViewportDock(ViewportNode *node);
    Viewport *getViewport();

public slots:
    void adjust(bool vis);

private:
    Viewport *viewport;
    ViewportNode *node;
};

#endif /* end of include guard: VIEWPORT */
