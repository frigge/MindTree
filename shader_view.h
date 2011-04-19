#ifndef SHADER_VIEW_H
#define SHADER_VIEW_H

#include "QGraphicsView"
#include "QGraphicsScene"
#include "QMainWindow"

class Shader_View : public QGraphicsView
{
public:
    Shader_View(QMainWindow *parent);
    QMainWindow *frgauthor;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void scaleView(qreal scaleFactor);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:

};

#endif // SHADER_VIEW_H
