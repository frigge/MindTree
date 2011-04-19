#ifndef NODELIB_H
#define NODELIB_H

#include <QTreeWidget>
#include "QGraphicsScene"
#include "scenewidgetcontainer.h"
#include "QDir"
#include "QMenu"

class NodeLib : public QTreeWidget
{
    Q_OBJECT
public:
    explicit NodeLib(QPointF atPos, QWidget *parent = 0);
    ~NodeLib();
    void update();
    void show_lib(QPointF atPos);
    void setScene(QGraphicsScene *scene);
    virtual bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
    SceneWidgetContainer *container;

signals:

public slots:
    void remove();
    void addFolder();
    void renamed(QTreeWidgetItem *item);
    void resizeLib();

private:
    float getLib_height();
    QGraphicsScene *space;
    QGraphicsProxyWidget *nodelib_proxy;
    QList<QTreeWidgetItem *> addNodeItems(QDir dir, QTreeWidgetItem *parent = 0);
    void createContextMenu();
    QPoint mousePos;
    void moveNode(QDir source, QDir destination);

    void addBuildInNodes();
};

#endif // NODELIB_H
