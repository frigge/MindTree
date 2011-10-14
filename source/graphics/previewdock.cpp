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

#include "previewdock.h"
#include "QDockWidget"
#include "QGLWidget"
#include "QWheelEvent"
#include "QContextMenuEvent"
#include "math.h"
#include "source/graphics/shaderpreview.h"

PreviewView::PreviewView(QDockWidget *parent)
    : QGraphicsView(parent)
{
    QGLWidget *qgl = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    setRenderHint(QPainter::Antialiasing);
    setViewport(qgl);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setScene(new QGraphicsScene);

    cMenu = new QMenu();
    QAction *refreshAction = cMenu->addAction("Refresh");
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(render()));
}

void PreviewView::contextMenuEvent(QContextMenuEvent *event)
{
    cMenu->exec(event->globalPos());
}

void PreviewView::mousePressEvent(QMouseEvent *event)    
{
    if(event->button() == Qt::LeftButton)
        setDragMode(ScrollHandDrag);
    else
        setDragMode(NoDrag);
    QGraphicsView::mousePressEvent(event);
}

void PreviewView::mouseReleaseEvent(QMouseEvent *event)    
{
    setDragMode(NoDrag);
    QGraphicsView::mouseReleaseEvent(event);
}

void PreviewView::setPreview(DShaderPreview *dprev)    
{
    prev = dprev;
    prev->getTimer()->connect(prev->getTimer(), SIGNAL(timeout()), this, SLOT(updatePixmap()));
    prev->getTimer()->connect(prev->getRenderProcess(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(updatePixmap()));
    if(!scene()->items().isEmpty())
    {
        QGraphicsItem *item = scene()->items().first();
        scene()->removeItem(item);
        delete item;
    }
    prevPix = scene()->addPixmap(QPixmap::fromImage(QImage(dprev->getImageFile())));
    scene()->setSceneRect(-500, -500, 1000, 1000);
}

void PreviewView::render()    
{
    prev->render(); 
}

void PreviewView::updatePixmap()    
{
    prevPix->setPixmap(QPixmap::fromImage(QImage(prev->getImageFile())));
}

void PreviewView::drawBackground(QPainter *painter, const QRectF &rect)
{
//    QRadialGradient *bggrad = new QRadialGradient(0, 0, rect.width());
//    bggrad->setColorAt(0, QColor(50, 50, 50));
//    bggrad->setColorAt(1, QColor(20, 20, 20));
//    painter->setBrush(QBrush(*bggrad));
    painter->setBrush(QBrush(QColor(70, 70, 70)));
    painter->drawRect(rect);
};

void PreviewView::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, -event->delta() / 240.0));
};

void PreviewView::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.01|| factor > 3)
        return;

    scale(scaleFactor, scaleFactor);
};
