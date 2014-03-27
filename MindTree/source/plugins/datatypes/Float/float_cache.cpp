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

#include "float_cache.h"

using namespace MindTree;
using namespace MindTree::Cache::Float;

//void floatValue(DataCache* cache)    
//{
//    cache->cache(cache->getNode()->getInSockets().at(0));
//}
//
//void intValue(DataCache* cache)    
//{
//    cache->cache(cache->getNode()->getInSockets().at(0));
//}
//
//void getLoopedCache(DataCache* cache)    
//{
//    //LoopSocketNode *ls = cache->getStart()
//    //    ->getNode()->getDerived<LoopSocketNode>();
//    //if(ls->getContainer()->getNodeType() == FOR) {
//    //    if(ls->getContainer()->getSocketOnContainer(cache->getStart()) 
//    //       == ls->getContainer()->getInSockets().at(2)) {
//    //        cache->data = LoopCacheControl::loop(
//    //                            ls->getContainer()
//    //                                ->getDerivedConst<LoopNode>())->getStep();
//    //    }
//    //    else{
//    //        cache->cache(ls->getContainer()
//    //                     ->getSocketOnContainer(cache->getStart())->toIn());
//    //    }
//    //}
//}
//
////void math(eMathOp op)    
////{
////    DinSocketList insockets = getNode()->getInSockets();
////    int i=0;
////    foreach(DinSocket *socket, insockets){
////        if(i == 0) {
////            int tmpdata = cache(socket);
////        }
////        else {
////            if(!socket->getCntdSocket() && socket->getType() == VARIABLE)
////                continue;
////
////            double tmpdata;
////            cache(socket);
////            switch(op)
////            {
////                case OPADD:
////                    tmpdata += data.getData<double>();
////                    break;
////                case OPSUBTRACT:
////                    tmpdata -= data.getData<double>();
////                    break;
////                case OPMULTIPLY:
////                    tmpdata *= data.getData<double>();
////                    break;
////                case OPDIVIDE:
////                    tmpdata /= data.getData<double>();
////                    break;
////            }
////        }
////        i++;
////    }
////}
//
//void modulo(DataCache* cache)
//{
//    //DinSocketList insockets = getNode()->getInSockets();
//    //cache(insockets.at(0));
//    //double val = data;
//    //cache(insockets.at(1));
//    //int div = data;
//
//    //data = val % div;
//}
//
//void container(DataCache* cache)    
//{
//    const ContainerNode *node = cache->getStart()->getNode()
//        ->getDerivedConst<ContainerNode>();
//    cache->cache(node->getSocketInContainer(cache->getStart())->toIn());
//}
//
////void stepup(DataCache* cache)    
////{
////    const ContainerNode *node = cache->getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
////    if(node->getNodeType() == FOREACHNODE
////        &&cache->getStart() == cache->getStart()->getNode()->getOutSocketLlist()->getLLsocketAt(0)->socket) {
////        cache->data = ((VectorForeachCacheThread*)QThread::currentThread())->getStep();
////        return;
////    }
////    cache->cache(node->getSocketOnContainer(cache->getStart())->toIn());
////}
//
