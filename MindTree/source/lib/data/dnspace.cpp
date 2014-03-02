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


#include "boost/python.hpp"
#include "data/project.h"
#include "data/frg.h"
#include "data/signal.h"
#include "dnspace.h"

using namespace MindTree;

DNSpace::Iterator begin(DNSpace *space)
{
    return DNSpace::Iterator(space, 0);
}

DNSpace::Iterator end(DNSpace *space)
{
    return DNSpace::Iterator(space, space->getNodeCnt()-1);
}

DNSpace::Iterator::Iterator(const DNSpace *space, int pos)
    : pos(pos), space(space)
{
}

bool DNSpace::Iterator::operator!=(const DNSpace::Iterator &other)    const
{
    return (other.pos == pos && other.space == space);
}

const DNSpace::Iterator& DNSpace::Iterator::operator++()    
{
    ++pos;
    return *this; 
}

DNode* DNSpace::Iterator::operator*()    const
{
    return space->getNodes().at(pos);
}

DNSpace::DNSpace()
    : isCSpace(false)
{
    FRG::CurrentProject->registerSpace(this);
    Signal::mergeSignals<DNode*>("addNode", "graphChanged");
}

DNSpace::DNSpace(const DNSpace &space)
    : name(space.getName()), isCSpace(false)
{
    for(auto *node : space.getNodes())
    {
        DNode *copy = node->clone();
        addNode(copy);
    }
}

DNSpace::~DNSpace()
{
    FRG::CurrentProject->unregisterSpace(this);
    //try to clear all links before deleting nodes
    for(auto *node : getNodes())
        for(auto *socket : node->getInSockets())
            socket->clearLink();

    for(auto *node : getNodes())
        delete node;
}

// QDataStream & MindTree::operator<<(QDataStream &stream, DNSpace *space)
// {
//     stream<<space->getName();
//     stream<<(qint16)space->getNodeCnt();
//     foreach(DNode *node, space->getNodes())
//         stream<<node;
// }

//QDataStream & MindTree::operator>>(QDataStream &stream, DNSpace **space)
//{
//    DNSpace *newspace = 0;
//    newspace = new DNSpace();
//    *space = newspace;
//    qint16 nodecnt;
//    QString name;
//    stream>>name;
//    newspace->setName(name);
//    stream>>nodecnt;
//    DNode_ptr node = 0;
//    for(int i = 0; i<nodecnt; i++)
//    {
//        stream>>&node;
//        newspace->addNode(node.get());
//    }
//}

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
    if(!node)return;
    node->setSpace(this);

    if(std::find(nodes.begin(), nodes.end(), node) != nodes.end()) return;
    nodes.push_back(node);

    MT_SIGNAL_EMITTER(node);
    MT_CUSTOM_SIGNAL_EMITTER("addNode", node);
}

void DNSpace::removeNode(DNode *node)
{
    unregisterNode(node);
    MT_CUSTOM_SIGNAL_EMITTER("removeNode", node);
    delete node;
}

void DNSpace::unregisterNode(DNode *node)    
{
    nodes.erase(std::find(nodes.begin(), nodes.end(), node));
    node->setSpace(0);
}

uint DNSpace::getNodeCnt()
{
    return nodes.size();
}

NodeList DNSpace::getNodes()const
{
    return nodes;
}

std::string DNSpace::getName() const
{
		return name;
}

void DNSpace::setName(std::string value)
{
		name = value;
}

bool DNSpace::isContainerSpace() const
{
    return isCSpace;
}

void DNSpace::setContainerSpace(bool value)
{
    isCSpace = value;
}

ContainerSpace* DNSpace::toContainer()    
{
    return dynamic_cast<ContainerSpace*>(this);
}

void DNSpace::addInfoBox(DInfoBox *box)    
{
    infos.push_back(box);
}

void DNSpace::removeInfoBox(DInfoBox *box)
{
    infos.erase(std::find(infos.begin(), infos.end(), box)); 
}

std::vector<DInfoBox*> DNSpace::getInfoBoxes()    
{
    return infos; 
}

ContainerSpace::ContainerSpace()
    : node(0), parentSpace(0)
{
    setContainerSpace(true);
}

ContainerSpace::ContainerSpace(const ContainerSpace &space)
    : DNSpace(space)
{
    setContainerSpace(true);
}

ContainerSpace::~ContainerSpace()
{
}

DNSpace* ContainerSpace::getParent()    
{
    return parentSpace;
}

//QDataStream & MindTree::operator>>(QDataStream &stream, ContainerSpace **space)
//{
//    ContainerSpace *newspace = 0;
//    newspace = new ContainerSpace();
//    *space = newspace;
//    qint16 nodecnt;
//    QString name;
//    stream>>name;
//    newspace->setName(name);
//    stream>>nodecnt;
//    DNode_ptr node = 0;
//    for(int i = 0; i<nodecnt; i++)
//    {
//        stream>>&node;
//        newspace->addNode(node.get());
//    }
//}

ContainerNode* ContainerSpace::getContainer()
{
    return node;
}

void ContainerSpace::setContainer(ContainerNode* value)
{
    node = value;
    parentSpace = node->getSpace();
}
