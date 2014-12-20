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

#include "source/plugins/datatypes/Object/object.h"
#include "data/nodes/nodetype.h"
#include "../../render/glwrapper.h"
#include "../../render/rendermanager.h"

namespace MindTree {
namespace GL {
    class GridRenderer;
    class RenderConfigurator;
}
}

class Widget3DManager;

class Viewport : public QGLWidget
{
    Q_OBJECT
public:
    Viewport();
    ~Viewport();
    void setStartSocket(MindTree::DoutSocket *socket);
    MindTree::DoutSocket* getStartSocket();

    MindTree::GL::RenderManager* getRenderManager();
    MindTree::GL::RenderPass* getPixelPass();
    void setOption(const std::string &key, MindTree::Property value);

//SLOTS:
    Q_SLOT void setShowPoints(bool b);
    Q_SLOT void setShowEdges(bool b);
    Q_SLOT void setShowPolygons(bool b);
    Q_SLOT void setShowFlatShading(bool b);
    Q_SLOT void setShowGrid(bool b);
    Q_SLOT void changeCamera(QString cam);
    Q_SLOT void setData(std::shared_ptr<Group> value);
//---------------

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

    void initializeGL() override;
    void resizeGL(int, int) override;
    void paintGL() override;

    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void transform(QPointF mpos);
    void checkTransform();
    void rotateView(float xdist, float ydist);
    void panView(float xdist, float ydist);
    void zoomView(float xdist, float ydist);
    void mouseToWorld();
    void drawFps();

private:
    std::shared_ptr<Camera> activeCamera, defaultCamera;

    std::unique_ptr<MindTree::GL::RenderConfigurator> _renderConfigurator;
    std::shared_ptr<Widget3DManager> _widgetManager;

    static std::vector<Viewport*> _viewports;

    QPointF lastpos;
    QPointF winClickPos;
    glm::vec3 mouseDistToObj;
    bool rotate, pan, zoom;
    bool selectionMode;
    bool _showGrid;
    unsigned short transformMode;
};

#endif /* end of include guard: VIEWPORT */
