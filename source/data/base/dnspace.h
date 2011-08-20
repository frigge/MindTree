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

#ifndef DNSPACE_H
#define DNSPACE_H

#include "QList"
#include "QString"

#include "source/data/nodes/data_node.h"
#include "source/graphics/nodelink.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/undo/frg_generic_undo.h"

class DNSpace : public FRGUndoBase
{
public:
    DNSpace();
    DNSpace(DNSpace* space);
    ~DNSpace();
    void addNode(DNode *node);
    void removeNode(DNode *node);
    void unregisterNode(DNode *node);
    QString getName();
	void setName(QString value);
    bool operator==(DNSpace &space);
    bool operator!=(DNSpace &space);

    VNSpace *getSpaceVis();
    void setSpaceVis(VNSpace*);

    qint16 getNodeCnt();
    NodeList getNodes();

    bool isContainerSpace();
    void setContainerSpace(bool value);
    ContainerSpace* toContainer();

private:
    VNSpace *spaceVis;
    QString name;
    QList<DNode*> nodes;
    bool isCSpace;
};

QDataStream & operator<<(QDataStream &stream, DNSpace *space);
QDataStream & operator>>(QDataStream &stream, DNSpace **space);

class ContainerSpace : public DNSpace
{
public:
    ContainerSpace();
    ContainerSpace(ContainerSpace* space);
    ContainerNode *getContainer();
    void setContainer(ContainerNode *node);

private:
    ContainerNode *node;
};

QDataStream & operator>>(QDataStream &stream, ContainerSpace **space);

#endif // DNSPACE_H
