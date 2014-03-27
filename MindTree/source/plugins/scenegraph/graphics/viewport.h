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
#include "QGLBuffer"
#include "QTimer"
#include "QThread"

#include "source/lib/mindtree_core.h"
#include "source/plugins/datatypes/Object/object.h"
#include "../../render/glwrapper.h"
#include "../../render/render.h"

class Viewport;

class Viewport : public QGLWidget
{
    Q_OBJECT
public:
    Viewport();
    ~Viewport();
    void setStartSocket(MindTree::DoutSocket *socket);
    MindTree::DoutSocket* getStartSocket();

public slots:
    void render(MindTree::DNode *node);
    void render();
    void repaint();
    void setShowPoints(bool b);
    void setShowEdges(bool b);
    void setShowPolygons(bool b);
    void setShowFlatShading(bool b);
    void changeCamera(QString cam);
    //void setSelectedNode();
    AbstractTransformableNode* getSelectedNode();
    glm::vec3 getCamPivot();

    void setData(std::shared_ptr<Group> value);

signals:
    void sceneUpdated();
    void nodeChanged(MindTree::DNode*);

protected:
    void resizeGL(int width, int height);
    void paintGL();
    void initializeGL();
    void drawGrid();
    void drawScene();
    void drawOrigin();
    void transform(QPointF mpos);
    void checkTransform();
    void drawTransformable(std::shared_ptr<AbstractTransformable> transformable);
    void drawAxisGizmo();
    void rotateView(float xdist, float ydist);
    void panView(float xdist, float ydist);
    void zoomView(float xdist, float ydist);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseToWorld();
    void drawFps();

private:
    AbstractTransformableNode *selectedNode;
    std::shared_ptr<Camera> activeCamera, defaultCamera;
    std::unique_ptr<MindTree::GL::QtContext> ctx;
    std::unique_ptr<MindTree::GL::RenderManager> rendermanager;

    QHash<MeshData*, QGLBuffer*> buffer_hash;

    QGLBuffer grid;
    QPointF lastpos;
    QPointF winClickPos;
    QTimer timer;
    glm::vec3 mouseDistToObj;
    bool rotate, pan, zoom;
    bool selectionMode;
    time_t start, end;
    unsigned short transformMode;
};

#endif /* end of include guard: VIEWPORT */
