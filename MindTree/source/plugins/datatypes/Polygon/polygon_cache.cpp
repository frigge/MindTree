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

#include "polygon_cache.h"
#include "cstdlib"
#include "ctime"
#include "iostream"
#include "../Int/int_cache.h"
//#include "source/data/base/frg.h"
//#include "source/data/base/project.h"

using namespace MindTree;

PolygonCache::PolygonCache(const DoutSocket *socket)
    : DataCache(socket)
{
    setType(DataCache::POLYGONCACHE);
    if(getStart())
        cacheInputs();
}

PolygonCache::~PolygonCache()
{
    clear();
}

PolygonCache* PolygonCache::getDerived()    
{
    return this;
}

void PolygonCache::composePolygon()    
{
    DinSocketList insockets = getNode()->getInSockets();
    Polygon p;
    std::vector<uint> verts;
    for(auto *socket : insockets){
        if(!socket->getCntdSocket())continue;
        verts.push_back(cacheForeign<int>(socket));
    }
    p.set(verts);
    data = p;
}

void PolygonCache::clear()    
{
}

void PolygonCache::composeArray()    
{
    //DinSocketList insockets = getStart()->getNode()->getInSockets();
    //int startsize = insockets.size() - 1;
    //data = new PolygonList;
    //PolygonCache *pc=0;
    //foreach(DinSocket *socket, insockets){
    //    if(!socket->getCntdSocket())continue;
    //    pc = new PolygonCache(socket);
    //    data->append(pc->getSingleData());
    //    pc->setOwner(true);
    //    delete pc;
    //}
}

void PolygonCache::container()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
    cache(node->getSocketInContainer(getStart())->toIn());
}

void PolygonCache::stepup()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
    cache(node->getSocketOnContainer(getStart())->toIn());
}

void PolygonCache::forloop()    
{
    const ForNode *node = getStart()->getNode()->getDerivedConst<ForNode>(); 
    DinSocket *start, *end, *step;
    DinSocketList insockets = node->getInSockets();

    start = insockets.at(0);
    end = insockets.at(1);
    step = insockets.at(2);

    int startval, endval, stepval;

    startval = cacheForeign<int>(start);
    endval = cacheForeign<int>(end);
    stepval = cacheForeign<int>(step);

    const LoopSocketNode *innode, *outnode;
    innode = node->getInputs()->getDerivedConst<LoopSocketNode>();
    outnode = node->getOutputs()->getDerivedConst<LoopSocketNode>();

    double stepping;
    const DinSocket *lin = node->getSocketInContainer(getStart())->toIn();
    LoopCache *c = LoopCacheControl::loop(node);
    PolygonCache *pcache = 0;
    for(stepping = startval; stepping < endval; stepping += stepval) {
        c->setStep(stepping);
        cache(lin);
        c->addData(this);
    }

    PolygonCache *finalpc = (PolygonCache*)c->getCache();
    if(finalpc) 
        data = finalpc->data;

    LoopCacheControl::del(node);
}

void PolygonCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    PolygonCache* loopedCache = (PolygonCache*)LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getCache();
    if(loopedCache) {
        data = loopedCache->data;
    }
    
    else{
        cache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
    }
}

void PolygonCache::setArray()    
{
    //DNode *node = getStart()->getNode();
    //DinSocketList inSockets = node->getInSockets();
    //DinSocket *arrSocket = inSockets.at(0);
    //DinSocket *valSocket = inSockets.at(1);
    //DinSocket *indexSocket = inSockets.at(2);
    //
    //int index=0;
    //Polygon value;
    //IntCache *indexCache = new IntCache(indexSocket);
    //index = indexCache->getSingleData();
    //delete indexCache;
    //PolygonCache *arrcache = new PolygonCache(arrSocket);
    //data = arrcache->getData();
    //delete arrcache;
    //PolygonCache *valueCache = new PolygonCache(valSocket);
    //value = valueCache->getSingleData();
    //delete valueCache;

    //if(!data) data = new PolygonList;
    //(*data)[index] = value;
}

