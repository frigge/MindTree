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

#include "vector_cache.h"

#include "../Int/int_cache.h"
#include "../Float/float_cache.h"
#include "../Object/object.h"

using namespace MindTree;

VectorCache::VectorCache(const DoutSocket *socket)
    : DataCache(socket)
{
    setType(DataCache::VECTORCACHE);
    cacheInputs();
}

VectorCache::~VectorCache()
{
    //clear();
}

//void VectorCache::clear()
//{
//}
//
//VectorCache *VectorCache::getDerived()    
//{
//    return this;
//}
//
//void VectorCache::setArray()    
//{
////    DNode *node = getStart()->getNode();
////    DinSocketList inSockets = node->getInSockets();
////    DinSocket *arrSocket = inSockets.at(0);
////    DinSocket *valSocket = inSockets.at(1);
////    DinSocket *indexSocket = inSockets.at(2);
////    
////    int index=0;
////    Vector value;
////    index = cacheForeign<int>(indexSocket);
////    cache(arrSocket);
////    VectorCache *valueCache = new VectorCache(valSocket);
////    value = valueCache->getSingleData();
////    delete valueCache;
////
////    if(!data)data = new VertexList();
////    (*data)[index] = value;
//}
//
//void VectorCache::composeArray()    
//{
////    ComposeArrayNode *node = getStart()->getNode()->getDerived<ComposeArrayNode>();
////
////    int pos = 0;
////    data = new VertexList;
////    VectorCache *cache=0;
////    foreach(DinSocket *socket, node->getInSockets()){
////        if(pos == node->getInSockets().size() - 2)return;
////        cache = new VectorCache(socket);
////        (*data)[pos] = cache->getSingleData();
////        delete cache;
////        pos++;
////    }
//}
//
//void VectorCache::vectorValue()    
//{
//    const VectorValueNode *node = getStart()->getNode()->getDerivedConst<VectorValueNode>(); 
//    cache(node->getInSockets().first());
//}
//
//void VectorCache::floattovector()    
//{
//    const DNode *node = getStart()->getNode();
//    
//    DinSocket *x, *y, *z;
//    DinSocketList insockets = node->getInSockets();
//   
//    x = insockets.at(0); 
//    y = insockets.at(1); 
//    z = insockets.at(2); 
//
//    double xval, yval, zval;
//
//    xval = cacheForeign<double>(x);
//    yval = cacheForeign<double>(y);
//    zval = cacheForeign<double>(z);
//
//    data = Vector(xval, yval, zval);
//    
//}
//
//void VectorCache::forloop()    
//{
//    const ForNode *node = getStart()->getNode()->getDerivedConst<ForNode>(); 
//    DinSocket *start, *end, *step;
//    DinSocketList insockets = node->getInSockets();
//
//    start = insockets.at(0);
//    end = insockets.at(1);
//    step = insockets.at(2);
//
//    int startval, endval, stepval;
//
//    startval = cacheForeign<int>(start);
//    endval = cacheForeign<int>(end);
//    stepval = cacheForeign<int>(step);
//
//    const LoopSocketNode *innode, *outnode;
//    innode = node->getInputs()->getDerivedConst<LoopSocketNode>();
//    outnode = node->getOutputs()->getDerivedConst<LoopSocketNode>();
//
//    double stepping;
//    const DinSocket *lin = node->getSocketInContainer(getStart())->toIn();
//    LoopCache *c = LoopCacheControl::loop(node);
//    VectorCache *vcache = 0;
//    for(stepping = startval; stepping < endval; stepping += stepval) {
//        c->setStep(stepping);
//        cache(lin);
//        c->addData(vcache);
//    }
//
//    VectorCache *finalvc = (VectorCache*)c->getCache();
//    if(finalvc) 
//        data = finalvc->getData();
//
//    LoopCacheControl::del(node);
//}
//
VectorForeachCacheThread::VectorForeachCacheThread(const DinSocket *socket, VertexList *array, int work_start, int work)
    : socket(socket), array(array), work_start(work_start), work(work), step(0)
{
    start();
}

VectorForeachCacheThread::~VectorForeachCacheThread()
{
    wait();
}

//void VectorForeachCacheThread::run()    
//{
//    auto v = std::make_shared<VectorCache>();
//    for(step=work_start; step<work_start + work; step++){
//        v->cache(socket);
//        (*array)[step] = v->getData();
//    }
//}
//
//int VectorForeachCacheThread::getStep()    
//{
//    return step; 
//}
//
//void VectorCache::foreachloop()    
//{
///*  
//    const ForeachNode *node = getStart()->getNode()->getDerivedConst<ForeachNode>(); 
//    DinSocket *arrayin;
//    DinSocketList insockets = node->getInSockets();
//    arrayin = insockets.at(0);
//
//    cache(arrayin);
//
//    int thread_count = QThread::idealThreadCount();
//    int thread_work = array_size/thread_count;
//    const DinSocket *newValSocket = node->getOutputs()->getInSockets().first();
//    VectorForeachCacheThread *threadptr[thread_count];
//
//    int curr_thread_work = 0;
//    for(int thread=0; thread<thread_count; thread++) {
//        if(thread == thread_count -1) 
//            curr_thread_work = thread_work + (array_size%thread_count);
//        else
//            curr_thread_work = thread_work;
//        threadptr[thread] = new VectorForeachCacheThread(newValSocket, data, thread*thread_work, curr_thread_work);
//    }
//    for(int thread=0; thread<thread_count; thread++)
//        delete threadptr[thread];
//*/
//}
//
//void VectorCache::getLoopedCache()    
//{
//    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
//    VectorCache* loopedCache = (VectorCache*)LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getCache();
//    if(loopedCache) 
//        data = loopedCache->getData();
//    
//    else{
//        cache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
//    }
//}
//
//void VectorCache::container()    
//{
//    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
//    cache(node->getSocketInContainer(getStart())->toIn());
//}
//
//void VectorCache::stepup()    
//{
//    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
//    cache(node->getSocketOnContainer(getStart())->toIn());
//}

VectorCacheThreaded::VectorCacheThreaded(const DinSocket *socket)
    : socket(socket), data(0)
{
    start();
}

VectorCacheThreaded::VectorCacheThreaded(const DAInSocket *socket)
    : socket(0), asocket(socket), data(0)
{
    start();
}

//void VectorCacheThreaded::run()    
//{
//   //VectorCache *cache=0;
//   //if(socket)
//   //    cache = new VectorCache(socket);
//   // else if(asocket)
//   //    cache = new VectorCache(asocket);
//   // if(cache) {
//   //     data = cache->getData(); 
//   //     delete cache;
//   // }
//}
//
//VertexList* VectorCacheThreaded::getData()    
//{
//    wait();
//    return data;
//}
//
