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

CallbackList::CallbackList()
    : block(false)
{
}

void CallbackList::setBlock(bool b)    
{
    block = b; 
}

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
    if(block)
        return;
    for(std::list<Callback*>::iterator cb = list.begin(); cb != list.end(); ++cb)
    {
        Callback *callback = *cb;
        callback->exec();
    }
}

void CallbackList::clear()    
{
    list.clear();
}

VNodeUpdateCallback::VNodeUpdateCallback(VNode *nodeVis)
    : nodeVis(nodeVis)
{}

void VNodeUpdateCallback::exec()    
{
    nodeVis->updateNodeVis();
}

ScpNameCB::ScpNameCB(DSocket *src, DSocket *dst)
    : src(src), dst(dst)
{
}

void ScpNameCB::exec()    
{
    if(dst->getName() != src->getName())
        dst->setName(src->getName());
}

ScpTypeCB::ScpTypeCB(DSocket *src, DSocket *dst)
    : src(src), dst(dst)
{
}

void ScpTypeCB::exec()    
{
    if(dst->getType() != src->getType())
        dst->setType(src->getType());
}

SsetToVarCB::SsetToVarCB(DSocket  *socket)
    : socket(socket)
{
}

void SsetToVarCB::exec()    
{
    socket->setType(VARIABLE);
}
