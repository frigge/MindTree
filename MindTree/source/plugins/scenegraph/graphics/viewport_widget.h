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

#ifndef VIEWPORT_WIDGET_F680V6U0

#define VIEWPORT_WIDGET_F680V6U0

#include "QObject"
#include "QWidget"
#include "QVBoxLayout"
#include "QToolBar"

#include "graphics/viewer.h"

class Viewport;
class ViewportWidget;
class QComboBox;

class ViewportViewer : public MindTree::Viewer
{
public:
    ViewportViewer(MindTree::DoutSocket *socket);
    virtual ~ViewportViewer();

    void update();
};

class ViewportWidget : public QWidget
{
    Q_OBJECT
public:
    ViewportWidget(ViewportViewer *viewer);
    ~ViewportWidget();

    QSize sizeHint() const;

    Viewport* getViewport();

    Q_SLOT void togglePolygons(bool b);
    Q_SLOT void toggleEdges(bool b);
    Q_SLOT void togglePoints(bool b);
    Q_SLOT void toggleFlatShading(bool b);
    Q_SLOT void toggleGrid(bool b);
    Q_SLOT void refillCamBox();
    Q_SLOT void setOutput(QString out);
    Q_SLOT void toggleDefaultLighting(bool value);

private:
    void createToolbar();
    void createMainLayout();

    ViewportViewer *_viewer;
    Viewport *_viewport;
    QComboBox *_camBox;
    QComboBox *_outputBox;
    QToolBar *_tools;
};

#endif /* end of include guard: VIEWPORT_WIDGET_F680V6U0 */
