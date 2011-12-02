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

#include "dnspace.h"

#include "QDebug"
#include "source/data/base/project.h"
#include "source/data/base/frg.h"

DNSpace::DNSpace()
    : spaceVis(0), isCSpace(false)
{
    FRG::CurrentProject->registerSpace(this);
}

DNSpace::DNSpace(DNSpace* space)
    : name(space->getName()), isCSpace(false)
{
    foreach(DNode *node, space->getNodes())
    {
        DNode *copy = DNode::copy(node);
        addNode(copy);
        FRG::CurrentProject->setNodePosition(copy, node->getPos());
    }
}

DNSpace::~DNSpace()
{
    FRG::CurrentProject->unregisterSpace(this);
    //try to clear all links before deleting nodes
    foreach(DNode *node, getNodes())
        foreach(DinSocket *socket, node->getInSockets())
            socket->clearLink();

    foreach(DNode *node, getNodes())
        delete node;
}


QDataStream & operator<<(QDataStream &stream, DNSpace *space)
{
    stream<<space->getName();
    stream<<(qint16)space->getNodeCnt();
    foreach(DNode *node, space->getNodes())
        stream<<node;
}

QDataStream & operator>>(QDataStream &stream, DNSpace **space)
{
    DNSpace *newspace = 0;
    newspace = new DNSpace();
    *space = newspace;
    qint16 nodecnt;
    QString name;
    stream>>name;
    newspace->setName(name);
    stream>>nodecnt;
    DNode *node = 0;
    for(int i = 0; i<nodecnt; i++)
    {
        stream>>&node;
        newspace->addNode(node);
    }
}

bool DNSpace::operator==(DNSpace &space)
{
    if(name != space.getName()
            ||getNodeCnt() != space.getNodeCnt())
        return false;

    foreach(DNode* node1, getNodes())
    {
        bool hasASimilar = false;
        foreach(DNode *node2, space.getNodes())
        {
            if(*node1 == *node2)
                hasASimilar = true;
        }
        if(!hasASimilar) return false;
    }
    return true;
}

bool DNSpace::operator!=(DNSpace &space)
{
    return(!(*this == space));
}

void DNSpace::addNode(DNode *node)
{
    node->setSpace(this);
    if(!nodes.contains(node))nodes.append(node);
}

void DNSpace::removeNode(DNode *node)
{
    unregisterNode(node);
    delete node;
}

void DNSpace::unregisterNode(DNode *node)    
{
    nodes.removeAll(node);
    node->setSpace(0);
    if(this == FRG::SpaceDataInFocus)
        node->deleteNodeVis();
}

VNSpace *DNSpace::getSpaceVis()
{
    return spaceVis;
}

void DNSpace::setSpaceVis(VNSpace *spaceVis)
{
    this->spaceVis = spaceVis;
}

qint16 DNSpace::getNodeCnt()
{
    return (qint16)nodes.size();
}

NodeList DNSpace::getNodes()
{
    return nodes;
}

QString DNSpace::getName()
{
		return name;
}

void DNSpace::setName(QString value)
{
		name = value;
}

bool DNSpace::isContainerSpace()
{
    return isCSpace;
}

void DNSpace::setContainerSpace(bool value)
{
    isCSpace = value;
}

ContainerSpace* DNSpace::toContainer()    
{
    return static_cast<ContainerSpace*>(this);
}

ContainerSpace::ContainerSpace()
    : DNSpace(), node(0)
{
    setContainerSpace(true);
}

ContainerSpace::ContainerSpace(ContainerSpace* space)
    : DNSpace(space)
{
    setContainerSpace(true);
}

QDataStream & operator>>(QDataStream &stream, ContainerSpace **space)
{
    ContainerSpace *newspace = 0;
    newspace = new ContainerSpace();
    *space = newspace;
    qint16 nodecnt;
    QString name;
    stream>>name;
    newspace->setName(name);
    stream>>nodecnt;
    DNode *node = 0;
    for(int i = 0; i<nodecnt; i++)
    {
        stream>>&node;
        newspace->addNode(node);
    }
}

ContainerNode* ContainerSpace::getContainer()
{
    return node;
}

void ContainerSpace::setContainer(ContainerNode* value)
{
    node = value;
}
