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

#include "source/graphics/base/vnspace.h"

class Shader_View : public QGraphicsView
{
public:
    Shader_View(QMainWindow *parent);
    void createSpace();
    VNSpace *getSpace();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    VNSpace *space;
};

#endif // SHADER_VIEW_H
