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

#ifndef VIEWER_DOCK_BASE_3EFJLTLO

#define VIEWER_DOCK_BASE_3EFJLTLO

#include "QDockWidget"
#include "QToolBar"
#include "QToolButton"
#include "boost/python.hpp"

namespace BPy = boost::python;

class QComboBox;
namespace MindTree
{
class ViewerDockHeaderWidget;
class DNode;
class ViewerNodeBase;

class ViewerTitleBarButton : public QToolButton
{
public:
    ViewerTitleBarButton(ViewerDockHeaderWidget *widget);
    QSize sizeHint() const;
};

class ViewerDockBase;

class ViewerDockHeaderWidget : public QToolBar
{
    Q_OBJECT
public:
    ViewerDockHeaderWidget(ViewerDockBase *viewer);
    QSize sizeHint()const;

public slots:
    void addToViewerBox(DNode*);
    void fillViewerBox();

private:
    void createButtons();
    void createViewerBox();
    bool comboBoxContains(QString name);

    QComboBox *viewerBox;
    ViewerTitleBarButton *pinButton;
    ViewerDockBase *viewer;
};

class DoutSocket;
class Viewer;
class ViewerDockBase : public QDockWidget
{
    Q_OBJECT
public:
    ViewerDockBase(QString name);
    virtual ~ViewerDockBase();
    void setViewer(Viewer *view);
    Viewer* getViewer();
    virtual void setFocusNode(DNode* node);
    virtual void exec(){}
    virtual void exec(DNode*){}
    void init();
    DoutSocket* getStart();
    void setStart(DoutSocket* value);
    void setPythonWidget(BPy::object widget);

public slots:
    void deleteThis(bool del);
    void toggleFloating();
    void destroy();
    void updateFocus();
    void update();
    void update(DNode* node);
    void adjust(bool vis);
    void setPinned(bool pin);

private:
    BPy::object pywidget;
    DoutSocket *startSocket;
    DNode *node;
    Viewer *viewer;
    bool pinned;
};

    
} /* MindTree */
#endif /* end of include guard: VIEWER_DOCK_BASE_3EFJLTLO */
