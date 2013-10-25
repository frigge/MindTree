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

#include "int_cache.h"
#include "../Vector/vector_cache.h"
#include "source/data/code_generator/outputs.h"

using namespace MindTree;

IntCache::IntCache(const DoutSocket *socket)
    : DataCache(socket)
{
    setType(DataCache::INTEGERCACHE);
    cacheInputs();
}

IntCache::~IntCache()
{
    //clear();
}

//bool IntCache::cache(const DinSocket *socket)    
//{
//    if(!DataCache::cache(socket))
//        data = socket->getProperty();
//    return true;
//}
//
//void IntCache::clear()
//{
//}
//
//IntCache *IntCache::getDerived()    
//{
//    return this;
//}
//
//void IntCache::composeArray()    
//{
//}
//
//void IntCache::intValue()    
//{
//    cache(getNode()->getInSockets().first());
//}
//
//void IntCache::getLoopedCache()    
//{
//    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
//    if(ls->getContainer()->getNodeType() == FOR) {
//        if(ls->getContainer()->getSocketOnContainer(getStart()) == ls->getContainer()->getInSocketLlist()->getLLsocketAt(2)->socket) {
//            data = LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getStep();
//        }
//        else{
//            cache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
//        }
//    }
//}
//
//void IntCache::math(eMathOp op)    
//{
//    DNode *node = getNode();
//    DSocketList *insockets = node->getInSocketLlist();
//    IntCache *fc=0;
//    LLsocket *first = insockets->getFirst();
//    LLsocket *soc = first;
//    int tmpdata;
//    while(soc) {
//        DinSocket *socket = soc->socket->toIn();
//        if(soc == first){
//            cache(socket);
//            tmpdata = data.getData<int>();
//        }
//        else {
//            if(!socket->getCntdSocket() && socket->getType() == VARIABLE) {
//                soc = soc->next;
//                continue;
//            }
//            switch(op) {
//                case OPADD:
//                    cache(socket);
//                    tmpdata += data.getData<int>();
//                    break;
//                case OPSUBTRACT:
//                    cache(socket);
//                    tmpdata -= data.getData<int>();
//                    break;
//                case OPMULTIPLY:
//                    cache(socket);
//                    tmpdata *= data.getData<int>();
//                    break;
//                case OPDIVIDE:
//                    cache(socket);
//                    tmpdata /= data.getData<int>();
//                    break;
//            }
//        }
//        soc = soc->next;
//    }
//    data = tmpdata;
//}
//
//void IntCache::modulo()
//{
//    DinSocketList insockets = getNode()->getInSockets();
//    cache(insockets.at(0));
//    int value = data.getData<int>();
//    cache(insockets.at(1));
//    int div = data.getData<int>();
//    data = value % div;
//}
//
//void IntCache::container()    
//{
//    const ContainerNode *node = getNode()->getDerivedConst<ContainerNode>();
//    cache(node->getSocketInContainer(getStart())->toIn());
//}
//
//void IntCache::stepup()    
//{
//    const ContainerNode *node = getNode()->getDerivedConst<SocketNode>()->getContainer();
//    if(node->getNodeType() == FOREACHNODE
//        &&getStart() == getStart()->getNode()->getOutSocketLlist()->getLLsocketAt(0)->socket) {
//        data = ((VectorForeachCacheThread*)QThread::currentThread())->getStep();
//        return;
//    }
//    cache(node->getSocketOnContainer(getStart())->toIn());
//}
//
//void IntCache::glShader()    
//{
//    const GLSLOutputNode *node = getStart()->getNode()->getDerivedConst<GLSLOutputNode>();
//    data = node->compile();
//}
//
//
