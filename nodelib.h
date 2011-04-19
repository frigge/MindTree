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
