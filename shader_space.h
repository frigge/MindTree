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


#ifndef SHADER_SPACE_H
#define SHADER_SPACE_H

#include "QGraphicsView"
#include "QGraphicsItem"
#include "QMenu"
#include "QObject"
#include "QPointF"

#include "nodelink.h"
#include "nodelib.h"
#include "newnodeeditor.h"
#include "node.h"

class Shader_Space : public QGraphicsScene
{

    Q_OBJECT
public:
    Shader_Space();
    void addLink(NodeLink *nlink, NSocket *final);
    void addLink(NodeLink *nlink);
    void moveLink(NodeLink *nlink);
    QPointF mousePos;
    NodeLib *nodelib;
    NewNodeEditor *nodeedit;
    void destroyContainer(QGraphicsItem *container);
    QString name;
    void setName(QString n){name = n;}
    NSocket *linksocket;
    NodeLink *newlink;
    void enterlinkNodeMode(NSocket *socket);
    void leavelinkNodeMode(QPointF finalpos);
    void enterEditNameMode();
    void leaveEditNameMode();
    bool editNameMode;

    void addNode(Node *node);
    int nodecnt;

    QList<NodeLink *> links;
    bool operator==(Shader_Space &space);
    bool operator!=(Shader_Space &space);

public slots:
    void shownodelib();
    void createNode();
    void removeNode(bool keeplinks);
    void removeSelectedNodes();
    void removeLink(NodeLink *link);

    void buildContainerNode();

    void changeScene(QGraphicsScene *scene);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

private:
    QMenu *NContextMenu;
    void ContextAddMenu();
    void updateLinks();
    Node *createFuncNode(QString filepath);
    Node *dropNode(QString filepath);
    QPointF getSelectedItemsCenter();
    QList<NodeLink *> getContainerOutLinks();
    QList<NodeLink *> getContainerInLinks();
    QList<NodeLink *> getContainerDataLinks();
    QList<Node *> copySelectedNodes(bool centered = false, QHash<NSocket *, NSocket *> *socketmapping = 0);
};

QDataStream & operator<<(QDataStream &stream, Shader_Space *space);
QDataStream & operator>>(QDataStream &stream, Shader_Space **space);

#endif // SHADER_SPACE_H
