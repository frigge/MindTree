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

#include "callbacks.h"

#include "source/graphics/nodes/graphics_node.h"
#include "source/data/nodes/data_node.h"

void CallbackList::add(Callback *cb)    
{
    list.push_back(cb);    
}

void CallbackList::remove(Callback *cb)    
{
    list.remove(cb); 
}

void CallbackList::operator()()    
{
    for(std::list<Callback*>::iterator cb = list.begin(); cb != list.end(); ++cb)
    {
        Callback *callback = *cb;
        callback->exec();
    }
}

VNodeUpdateCallback::VNodeUpdateCallback(VNode *nodeVis)
    : nodeVis(nodeVis)
{}

void VNodeUpdateCallback::exec()    
{
    nodeVis->updateNodeVis();
}

SNchangeNameCB::SNchangeNameCB(DSocket *socket)
    : socket(socket)
{}

void SNchangeNameCB::exec()    
{
    socket->setName(socket->getNode()->getDerived<SocketNode>()->getContainer()->getSocketInContainer(socket)->getName());
}

SNchangeTypeCB::SNchangeTypeCB(DSocket *socket)
    : socket(socket)
{}

void SNchangeTypeCB::exec()    
{
    socket->setType(socket->getNode()->getDerived<SocketNode>()->getContainer()->getSocketInContainer(socket)->getType());
}

SInTypeToOutTypeCB::SInTypeToOutTypeCB(DSocket *socket)
    : socket(socket)
{
}

void SInTypeToOutTypeCB::exec()    
{
    if(socket->getDir() == IN)
        socket->getNode()->getOutSockets().first()->setType(socket->getType());
}
