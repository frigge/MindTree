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
#include "data/signal.h"
#include "data/nodes/containernode.h"
#include "data/io.h"
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
{
    Signal::mergeSignals<DNode*>("addNode", "graphChanged");
}

DNSpace::DNSpace(const DNSpace &space)
    : name(space.getName())
{
    for(auto *node : space.getNodes())
    {
        DNode *copy = node->clone();
        addNode(copy);
    }
}

DNSpace::~DNSpace()
{
    //try to clear all links before deleting nodes
    for(auto *node : getNodes())
        for(auto *socket : node->getInSockets())
            socket->clearLink();

    for(auto *node : getNodes())
        removeNode(node);
}

bool DNSpace::operator==(DNSpace &space)
{
    if(name != space.getName()
            ||getNodeCnt() != space.getNodeCnt())
        return false;

    for(DNode* node1 : getNodes())
    {
        bool hasASimilar = false;
        for(DNode *node2 : space.getNodes())
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
    return false;
}

ContainerSpace* DNSpace::toContainer()    
{
    return dynamic_cast<ContainerSpace*>(this);
}

ContainerSpace::ContainerSpace()
    : node(nullptr), parentSpace(nullptr)
{
}

ContainerSpace::ContainerSpace(const ContainerSpace &space)
    : DNSpace(space)
{
}

ContainerSpace::~ContainerSpace()
{
}

DNSpace* ContainerSpace::getParent()    
{
    return parentSpace;
}

void ContainerSpace::setParentSpace(DNSpace *space)
{
    parentSpace = space;
}

bool ContainerSpace::isContainerSpace() const
{
    return true;
}

ContainerNode* ContainerSpace::getContainer()
{
    return node;
}

void ContainerSpace::setContainer(ContainerNode* value)
{
    node = value;
    parentSpace = node->getSpace();
}

IO::OutStream& MindTree::operator<<(IO::OutStream &stream, const DNSpace &space)
{
    stream.beginBlock("Space");
    stream << space.getName();
    stream << space.getNodes().size();
    for (const DNode* node : space.getNodes()) {
        stream << *node;
    }
    stream.endBlock("Space");
    return stream;
}

IO::InStream& MindTree::operator>>(IO::InStream &stream, DNSpace &space)
{
    stream.beginBlock("Space");
    int nodecnt = 0;
    std::string name;
    stream >> name;
    space.setName(name);
    stream >> nodecnt;

    bool isCont = space.isContainerSpace();

    if(isCont) {
        auto nodes = space.getNodes();
        auto* inNode = nodes[0];
        auto* outNode = nodes[1];

        NodeType t;
        stream.beginBlock("DNode");
        stream >> t;
        stream >> *inNode;
        stream.endBlock("DNode");
        stream.beginBlock("DNode");
        stream >> t;
        stream >> *outNode;
        stream.endBlock("DNode");
    }

    for(int i = isCont ? 2 : 0; i < nodecnt; i++) {
        stream.beginBlock("DNode");
        NodeType type;
        stream >> type;

        auto* node = NodeDataBase::createNodeByType(type);
        if(node) stream >> *node;
        stream.endBlock("DNode");

        space.addNode(node);
    }
    return stream;
}

