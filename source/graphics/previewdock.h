/*
    frg_Shader_Author Shader Editor, a Node-based Renderman Shading Language Editor
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

#ifndef PREVIEWDOCK_HDP4EB16

#define PREVIEWDOCK_HDP4EB16

#include "QGraphicsView"
#include "QDockWidget"
#include "QGraphicsScene"
#include "QMenu"

class QDockWidget;
class DShaderPreview;
class PreviewView;
class PreviewDock : public QDockWidget
{
    Q_OBJECT
public:
    PreviewDock(DShaderPreview *prev);
    virtual ~PreviewDock();

public slots:
    void adjustPreviewNode(bool vis);

private:
    DShaderPreview *preview;
    PreviewView *pview;
};
class DNode;

class PreviewView : public QGraphicsView
{
Q_OBJECT
public:
    PreviewView(QDockWidget *parent);
    void setPreview(DShaderPreview *dprev);

public slots:
    void updatePixmap();
    void render();
    void render(DNode* node);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void scaleView(qreal scaleFactor);
    void wheelEvent(QWheelEvent *event);

private:
    DShaderPreview *prev;
    QGraphicsPixmapItem *prevPix;
    QMenu *cMenu;
};
#endif /* end of include guard: PREVIEWDOCK_HDP4EB16 */
