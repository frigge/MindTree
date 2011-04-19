#ifndef SCENEWIDGETCONTAINER_H
#define SCENEWIDGETCONTAINER_H

#include <QGraphicsItem>
#include "QGraphicsProxyWidget"

class SceneWidgetCloseButton : public QGraphicsItem
{
public:
    QGraphicsItem *container;
    SceneWidgetCloseButton();
    void setParent(QGraphicsItem *parent);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class SceneWidgetContainer : public QGraphicsItem
{
public:
    enum {Type = UserType + 3};
    QPoint closepos;
    explicit SceneWidgetContainer();
    ~SceneWidgetContainer();
    void setProxy(QGraphicsProxyWidget *child);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QGraphicsProxyWidget *childproxy;
    int type() const {return Type;}

private:
    SceneWidgetCloseButton close;
};

#endif // SCENEWIDGETCONTAINER_H
