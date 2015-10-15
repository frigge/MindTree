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
#include "QTimer"
#include "QThread"

#include "source/plugins/datatypes/Object/object.h"
#include "data/nodes/nodetype.h"
#include "../../render/glwrapper.h"
#include "../../render/rendertree.h"

#include "QGLWidget"

namespace MindTree {
namespace GL {
    class GridRenderer;
    class RenderConfigurator;
}
}

class Widget3DManager;

class ViewportWidget;
class Viewport : public QGLWidget
{
    Q_OBJECT
public:
    Viewport(ViewportWidget *widget);
    ~Viewport();
    void setStartSocket(MindTree::DoutSocket *socket);
    MindTree::DoutSocket* getStartSocket();

    MindTree::GL::RenderTree* getRenderTree();
    MindTree::GL::RenderPass* getPixelPass();
    void setOption(const std::string &key, MindTree::Property value);

    std::vector<std::string> getCameras() const;

    void setOverride(std::string name);
    void clearOverrideOutput();
    MindTree::PropertyMap getSettings() const;

//SLOTS:
    Q_SLOT void setShowPoints(bool b);
    Q_SLOT void setShowEdges(bool b);
    Q_SLOT void setShowPolygons(bool b);
    Q_SLOT void setShowFlatShading(bool b);
    Q_SLOT void setShowGrid(bool b);
    Q_SLOT void changeCamera(std::string cam);
    Q_SLOT void setData(std::shared_ptr<Group> value);
    Q_SLOT void exitFullscreen();
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
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void transform(QPointF mpos);
    void checkTransform();
    void rotateView(float xdist, float ydist);
    void panView(float xdist, float ydist);
    void zoomView(float xdist, float ydist);
    void mouseToWorld();
    void drawFps();

private:
    friend class ViewportWidget;
    std::shared_ptr<Camera> activeCamera, defaultCamera;

    std::unique_ptr<MindTree::GL::RenderConfigurator> _renderConfigurator;
    std::shared_ptr<Widget3DManager> _widgetManager;

    static std::vector<Viewport*> _viewports;
    std::unordered_map<std::string, std::shared_ptr<Light>> _lights;
    std::unordered_map<std::string, std::shared_ptr<Camera>> _cameras;
    std::vector<std::string> _camNames;

    QPointF lastpos;
    QPointF winClickPos;
    glm::vec3 mouseDistToObj;
    bool rotate, pan, zoom;
    bool selectionMode;
    bool _showGrid;
    unsigned short transformMode;

    ViewportWidget *_viewportWidget;
};

#endif /* end of include guard: VIEWPORT */
