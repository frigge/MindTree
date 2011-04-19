#include "shader_view.h"
#include "QMouseEvent"
#include "math.h"
#include "QtGui"
#include "QtOpenGL/QtOpenGL"
#include "QtOpenGL/QGLWidget"

Shader_View::Shader_View(QMainWindow *parent)
{
    frgauthor = parent;
    QGLWidget *qgl = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    setRenderHint(QPainter::Antialiasing);
    setViewport(qgl);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void Shader_View::mousePressEvent(QMouseEvent *event)
{
        QGraphicsView::mousePressEvent(event);
};

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

void Shader_View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
};

void Shader_View::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
        setDragMode(RubberBandDrag);
    else
        QGraphicsView::mouseReleaseEvent(event);
};

void Shader_View::drawBackground(QPainter *painter, const QRectF &rect)
{
    QRadialGradient *bggrad = new QRadialGradient(0, 0, rect.width());
    bggrad->setColorAt(0, QColor(50, 50, 50));
    bggrad->setColorAt(1, QColor(20, 20, 20));
    painter->setBrush(QBrush(*bggrad));
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
