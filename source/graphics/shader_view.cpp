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

#include "shader_view.h"

#include "QMouseEvent"
#include "math.h"
#include "QtGui"
#include "QtOpenGL/QtOpenGL"
#include "QtOpenGL/QGLWidget"

Shader_View::Shader_View(QMainWindow *parent)
	: QGraphicsView(parent)
{
    QGLWidget *qgl = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    setViewport(qgl);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(BoundingRectViewportUpdate);
}

void Shader_View::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt)
        setDragMode(ScrollHandDrag);
    QGraphicsView::keyPressEvent(event);
};

void Shader_View::keyReleaseEvent(QKeyEvent *event)
{
    setDragMode(RubberBandDrag);
};

void Shader_View::mouseReleaseEvent(QMouseEvent *event)
{
    setDragMode(RubberBandDrag);
    QGraphicsView::mouseReleaseEvent(event);
};

void Shader_View::drawBackground(QPainter *painter, const QRectF &rect)
{
//    QRadialGradient *bggrad = new QRadialGradient(0, 0, rect.width());
//    bggrad->setColorAt(0, QColor(50, 50, 50));
//    bggrad->setColorAt(1, QColor(20, 20, 20));
//    painter->setBrush(QBrush(*bggrad));
    painter->setBrush(QBrush(QColor(40, 40, 40)));
    painter->drawRect(rect);
};

void Shader_View::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, -event->delta() / 240.0));
};

void Shader_View::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.01|| factor > 3)
        return;

    scale(scaleFactor, scaleFactor);
};

void Shader_View::createSpace()
{
    DNSpace *newspace = new DNSpace();
    space->moveIntoSpace(newspace);
}

VNSpace* Shader_View::getSpace()
{
    return space;
}

void Shader_View::paintEvent(QPaintEvent *event)    
{
    //QPaintEvent *newEvent = new QPaintEvent(event->region().boundingRect());
    QGraphicsView::paintEvent(event);
    //delete newEvent;
}
