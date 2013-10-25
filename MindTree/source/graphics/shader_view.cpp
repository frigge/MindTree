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
#include "QResizeEvent"
#include "math.h"
#include "QtGui"
#include "QtOpenGL/QtOpenGL"
#include "QtOpenGL/QGLWidget"
#include "iostream"

#include "source/graphics/base/vnspace.h"
#include "source/graphics/nodelib.h"
#include "data/frg.h"
#include "source/graphics/base/mindtree_mainwindow.h"
#include "data/project.h"

using namespace MindTree;

ChangeSpaceAction::ChangeSpaceAction(DNSpace *space, QObject *parent)
    : QAction(parent)
{
    new_space = space;
    connect(this, SIGNAL(triggered()), this, SLOT(emitTriggered()));
}

void ChangeSpaceAction::emitTriggered()
{
    emit triggered(new_space);
}

NodeGraphDock::NodeGraphDock()
    : QDockWidget("NodeGraph")
{
    setWidget(new NodeGraphWidget());
}

NodeGraphDock::~NodeGraphDock()
{
}

NodeGraphWidget::NodeGraphWidget()
    : QWidget(0),
    view(new Shader_View)
{
    FRG::Graph = this;
    VNSpace *space;
    QHBoxLayout *hlay = new QHBoxLayout();
    hlay->setSpacing(0);
    hlay->setContentsMargins(0, 0, 0, 0);
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    NodeLibWidget *nlw = new NodeLibWidget(0);
    setLayout(hlay);
    hlay->addWidget(splitter);
    splitter->addWidget(nlw);
    splitter->setStretchFactor(0, 1);
    splitter->setOpaqueResize(false);
    QWidget *graphWidget = new QWidget(splitter);
    QVBoxLayout *vlay = new QVBoxLayout();
    vlay->setSpacing(0);
    vlay->setContentsMargins(0, 0, 0, 0);
    graphWidget->setLayout(vlay);
    toolbar = new QToolBar("Node Path");
    QAction *rootAction = toolbar->addAction("/Root");
    connect(rootAction, SIGNAL(triggered()), this, SLOT(setRoot_Space()));
    vlay->addWidget(toolbar);
    vlay->addWidget(view);
    view->adjustViewRPrev();
}

NodeGraphWidget::~NodeGraphWidget()
{
}

void NodeGraphWidget::updateToolBar()
{
    foreach(QAction *action, toolbar->actions())
        toolbar->removeAction(action);

    QAction *rootAction = toolbar->addAction("/Root");
    connect(rootAction, SIGNAL(triggered()), this, SLOT(setRoot_Space()));

    while(!spaceActions.isEmpty())
        delete spaceActions.takeLast();

    foreach(DNSpace *space, spaces)
    {
        ChangeSpaceAction *chspace = new ChangeSpaceAction(space, toolbar);
        spaceActions.append(chspace);
        chspace->setText("/" + space->getName());
        toolbar->addAction(chspace);
        connect(chspace, SIGNAL(triggered(DNSpace*)), this, SLOT(moveIntoSpace(DNSpace*)));
    }
}

void NodeGraphWidget::setRoot_Space()
{
    view->getSpace()->moveIntoSpace(FRG::CurrentProject->getRootSpace());
    spaces.clear();
    updateToolBar();
}

void NodeGraphWidget::moveIntoSpace(DNSpace *space)
{
    view->getSpace()->moveIntoSpace(space);

    if(!spaces.isEmpty())
        while(spaces.contains(space))
            spaces.takeLast();
    spaces.append(space);
    updateToolBar();
}

Shader_View::Shader_View()
    : viewR(0),
      spaceR(0),
      drag(false),
      bottomWidget(0),
      topLeft(new QGraphicsRectItem),
      bottomRight(new QGraphicsRectItem)
{
    setScene(new VNSpace());
    QGLWidget *glWidget = new QGLWidget();
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setViewport(glWidget);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    topLeft->setRect(QRect(0, 0, 10, 10));
    topLeft->setBrush(QBrush(QColor(10, 10, 10, 120)));
    bottomRight->setRect(QRect(-10, -10, 10, 10));
    bottomRight->setBrush(QBrush(QColor(10, 10, 10, 120)));
    scene()->addItem(topLeft);
    scene()->addItem(bottomRight);
    updateSceneRect();
    setUpdatesEnabled(true);
    setDragMode(QGraphicsView::RubberBandDrag);
}

void Shader_View::adjustResizeItems()    
{
    QPointF tl(mapToScene(QPoint(0, 0)));
    QPointF br(mapToScene(width(), height()));

    std::cout<<"topLeft Item: "<<tl.x() << ", " << tl.y() << std::endl;
    std::cout<<"bottomRight Item: "<<br.x() << ", " << br.y() << std::endl;

    topLeft->setPos(tl);
    bottomRight->setPos(br);
}

void Shader_View::mousePressEvent(QMouseEvent *event)    
{
    if(event->button() == Qt::MiddleButton) {
        lastPos = event->pos();
        setDragMode(QGraphicsView::NoDrag);
        setCursor(Qt::ClosedHandCursor);
        drag = true;
    }
    QGraphicsView::mousePressEvent(event);
}

void Shader_View::mouseMoveEvent(QMouseEvent *event)    
{
    if(drag){
        adjustSceneRect(event->pos());
        testWidgets();
    }
    QGraphicsView::mouseMoveEvent(event);
    //adjustResizeItems();
    QPointF mpos = mapToScene(event->pos());
    if(FRG::Author->isMouseNodeGraphPos())
        FRG::Author->statusBar()->showMessage("MousePos: " + QString::number(mpos.x()) + ", " + QString::number(mpos.y()));
}

void Shader_View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    setCursor(Qt::ArrowCursor);
    setDragMode(QGraphicsView::RubberBandDrag);
    drag = false;
};

void Shader_View::resizeEvent(QResizeEvent *event)    
{
    updateSceneRect(); 
    adjustResizeItems();
}

void Shader_View::updateSceneRect()    
{
    adjustSceneRect(lastPos);
    //QRectF sceneRect = FRG::Space->sceneRect();
    //QRectF visRect = visibleRect();
    //if(sceneRect.width() < visRect.width())
    //    sceneRect.setWidth(visRect.width());
    //if(sceneRect.height() < visRect.height())
    //    sceneRect.setHeight(visRect.height());
    //FRG::Space->setSceneRect(sceneRect);
    //adjustViewRPrev();
}

QRectF Shader_View::visibleRect() 
{
    QPointF tl(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QPointF br = tl + viewport()->rect().bottomRight();
    QMatrix mat = matrix().inverted();
    return mat.mapRect(QRectF(tl,br));
}

void qPrintRect(QRectF rect)
{
    std::cout << "QRectF: TopLeft: (" << rect.left() << ", " << rect.top() << "), BottomRight: (" << rect.right() << ", " << rect.bottom() << ")" << std::endl;
}

void Shader_View::adjustSceneRect(QPoint mousePos)    
{
    QPointF tl(mapToScene(QPoint()));
    QPointF br(mapToScene(QPoint(width(), height())));
    //QRectF visRect(tl, br);
    QRectF visRect(visibleRect());

    QPointF mov = mapToScene(mousePos) - mapToScene(lastPos);
    mov *= -1;
    topLeft->setPos(topLeft->pos() + mov);
    bottomRight->setPos(bottomRight->pos() + mov);
    scene()->setSceneRect(scene()->itemsBoundingRect());

    QPointF center = visRect.center();
    QPointF newcenter(center+mov);
    centerOn(newcenter);
    adjustResizeItems();

    lastPos = mousePos;
    adjustViewRPrev();
    qPrintRect(visRect);
    qPrintRect(scene()->sceneRect());
}

void Shader_View::testWidgets()    
{
    if(!bottomWidget){
        bottomWidget = new QGraphicsRectItem();
        bottomWidget->setBrush(QBrush(QColor(10, 10, 10, 120)));
        bottomWidget->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        bottomWidget->setRect(QRect(0, 0, 100, 100));
        scene()->addItem(bottomWidget);
    }
    QRectF visRect = visibleRect();
    bottomWidget->setPos(visRect.bottomLeft());
}

void Shader_View::adjustViewRPrev()    
{
    QRectF origSR = scene()->sceneRect();
    QRectF origVR = visibleRect();

    float scale = 100;
    if(!spaceR) { 
        spaceR = new QGraphicsRectItem();
        spaceR->setBrush(QBrush(QColor(10, 10, 10, 120)));
        spaceR->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        scene()->addItem(spaceR);
    }
    if(!viewR) {
        viewR = new QGraphicsRectItem();
        viewR->setBrush(QBrush(QColor(60, 20, 20, 130)));
        viewR->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        viewR->setParentItem(spaceR);
    }

    float srw = origSR.width();
    float srh = origSR.height();
    float srr = srw > srh ? srh/srw : srw/srh;
    srw = srw > srh ? 1 : srr;
    srh = srh > srw ? 1 : srr;
    srw *= scale;
    srh *= scale;
    QRectF sr(0, 0, srw, srh);

    float vrw = origVR.width()/origSR.width();
    float vrh = origVR.height()/origSR.height();
    float vpx = (origVR.x() - origSR.x()) / origSR.width();
    float vpy = (origVR.y() - origSR.y()) / origSR.height();
    vrw *= scale;
    vrh *= scale;

    srw/=scale;
    srh/=scale;
    QRectF vr(0, 0, vrw*srw, vrh*srh);

    viewR->setPos(vpx * scale*srw, vpy * scale*srh);
    spaceR->setPos(mapToScene(QPoint(width(), height()) - QPoint(150, 150)));
    spaceR->setRect(sr);
    viewR->setRect(vr);
}

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
    if(dragMode() == ScrollHandDrag) return;
    scaleView(pow((double)2, -event->delta() / 240.0));
};

void Shader_View::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.01|| factor > 3)
        return;

    scale(scaleFactor, scaleFactor);
    updateSceneRect();
};

void Shader_View::createSpace()
{
    DNSpace *newspace = new DNSpace();
    ((VNSpace*)scene())->moveIntoSpace(newspace);
}

VNSpace* Shader_View::getSpace()
{
    return (VNSpace*)scene();
}

void Shader_View::paintEvent(QPaintEvent *event)    
{
    //QPaintEvent *newEvent = new QPaintEvent(event->region().boundingRect());
    QGraphicsView::paintEvent(event);
    //delete newEvent;
}
