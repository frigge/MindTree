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
    : block(false), cblist(0)
{
}

void CallbackList::setBlock(bool b)    
{
    block = b; 
}

void CallbackList::add(Callback *cb)    
{
    if(!cblist){
        cblist = new CBstruct;
        cblist->next = 0;
        cblist->cb = cb;
        return;
    }
    CBstruct *f = cblist;
    while(f->next) f = f->next;
    f->next = new CBstruct;
    f->next->cb = cb;
    f->next->next = 0;
}

void CallbackList::remove(Callback *cb)    
{
    if(!cblist ||!cb) return;
    CBstruct *f = cblist;
    if(f->cb == cb){
        cblist = f->next;
        delete f;
        return;
    }

    while(f->next){
        if(f->next->cb == cb)
            break;
        f = f->next;
    }

    if(!f->next || f->next->cb != cb) return;
    CBstruct *toremove = f->next;
    f->next = toremove->next;
    delete toremove;
}

void CallbackList::operator()()    
{
    if(block || !cblist)
        return;

    CBstruct *f = cblist;
    do{
        f->cb->exec();
        f = f->next;
    }while(f);
}

void CallbackList::clear()    
{
    CBstruct *f = cblist;
    while(f){
        CBstruct *tmp = f;
        f = f->next;
        delete tmp->cb;
        delete tmp;
    }
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
