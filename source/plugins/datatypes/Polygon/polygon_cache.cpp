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

//void composePolygon(DataCache *cache)    
//{
//    DinSocketList insockets = cache->getNode()->getInSockets();
//    Polygon p;
//    std::vector<uint> verts;
//    for(int i=0; i<insockets.size(); i++){
//        verts.push_back(cache->getData(i).getData<int>());
//    }
//    p.set(verts);
//    cache->pushData(p);
//}
//
//void composeArray(DataCache *cache)    
//{
//    //DinSocketList insockets = getStart()->getNode()->getInSockets();
//    //int startsize = insockets.size() - 1;
//    //data = new PolygonList;
//    //PolygonCache *pc=0;
//    //foreach(DinSocket *socket, insockets){
//    //    if(!socket->getCntdSocket())continue;
//    //    pc = new PolygonCache(socket);
//    //    data->append(pc->getSingleData());
//    //    pc->setOwner(true);
//    //    delete pc;
//    //}
//}
//
//void container(DataCache *cache)    
//{
//    const ContainerNode *node = cache->getStart()->getNode()->getDerivedConst<ContainerNode>();
//    cache->cache(node->getSocketInContainer(cache->getStart())->toIn());
//}
//
//void stepup(DataCache *cache)    
//{
//    const ContainerNode *node = cache->getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
//    cache->cache(node->getSocketOnContainer(cache->getStart())->toIn());
//}
//
//void forloop(DataCache *cache)    
//{
//    const ForNode *node = cache->getNode()->getDerivedConst<ForNode>(); 
//
//    int startval, endval, stepval;
//
//    //cache start, end and step values
//    startval = cache->getData(0).getData<int>();
//    endval = cache->getData(1).getData<int>();
//    stepval = cache->getData(2).getData<int>();
//
//    //get corresponding entries inside the loop node
//    const LoopSocketNode *innode, *outnode;
//    innode = node->getInputs()->getDerivedConst<LoopSocketNode>();
//    outnode = node->getOutputs()->getDerivedConst<LoopSocketNode>();
//
//    double stepping;
//    const DinSocket *lin = node->getSocketInContainer(cache->getStart())->toIn();
//    LoopCache *c = LoopCacheControl::loop(node);
//
//    for(stepping = startval; stepping < endval; stepping += stepval) {
//        c->setStep(stepping);
//        //cache->cache(lin);
//    }
//
//    LoopCacheControl::del(node);
//}
//
//void getLoopedCache(DataCache *cache)    
//{
//    //LoopSocketNode *ls = cache->getStart()->getNode()->getDerived<LoopSocketNode>();
//    //PolygonCache* loopedCache = (PolygonCache*)LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getCache();
//    //if(loopedCache) {
//    //    cache->data = loopedCache->data;
//    //}
//    //
//    //else{
//    //    cache->cache(ls->getContainer()->getSocketOnContainer(cache->getStart())->toIn());
//    //}
//}
//
//void setArray(DataCache *cache)
//{
//    //DNode *node = getStart()->getNode();
//    //DinSocketList inSockets = node->getInSockets();
//    //DinSocket *arrSocket = inSockets.at(0);
//    //DinSocket *valSocket = inSockets.at(1);
//    //DinSocket *indexSocket = inSockets.at(2);
//    //
//    //int index=0;
//    //Polygon value;
//    //IntCache *indexCache = new IntCache(indexSocket);
//    //index = indexCache->getSingleData();
//    //delete indexCache;
//    //PolygonCache *arrcache = new PolygonCache(arrSocket);
//    //data = arrcache->getData();
//    //delete arrcache;
//    //PolygonCache *valueCache = new PolygonCache(valSocket);
//    //value = valueCache->getSingleData();
//    //delete valueCache;
//
//    //if(!data) data = new PolygonList;
//    //(*data)[index] = value;
//}
//
