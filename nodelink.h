#ifndef NODELINK_H
#define NODELINK_H

#include "QGraphicsItem"
#include "QGraphicsView"
#include "QMenu"
#include "QObject"

#include "node.h"

class NodeLink : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    NodeLink(QPointF startP, QPointF endP);
    NodeLink();
    ~NodeLink();
    NSocket *inSocket, *outSocket;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setlink(QPointF endP);
    void setlink(NSocket *in, NSocket *out);
    void updateLink();

    static bool isCompatible(NSocket *first, NSocket *last);

public slots:
    void remove();

signals:
    void removeLink(NodeLink *link);

private:
    QPointF in, out, c1, c2;
    QPainterPath drawPath();
    QMenu *cMenu;
    void initNodeLink();
    QPointF getmiddle(QPointF p1, QPointF p2);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

};

QDataStream & operator<<(QDataStream &stream, NodeLink *nlink);
QDataStream & operator>>(QDataStream &stream, NodeLink **nlink);

#endif // NODELINK_H
