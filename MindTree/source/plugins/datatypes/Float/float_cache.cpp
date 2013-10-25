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
#include "../Int/int_cache.h"
#include "../Vector/vector_cache.h"

using namespace MindTree;
FloatCache::FloatCache(const DoutSocket *socket)
    : DataCache(socket)
{
}

FloatCache::~FloatCache()
{
}

//void FloatCache::clear()
//{
//}
//
//FloatCache* FloatCache::getDerived()    
//{
//    return this;
//}
//
//void FloatCache::floatValue()    
//{
//    cache(getNode()->getInSockets().first());
//}
//
//void FloatCache::intValue()    
//{
//    cache(getNode()->getInSockets().first());
//}
//
//void FloatCache::getLoopedCache()    
//{
//    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
//    if(ls->getContainer()->getNodeType() == FOR) {
//        if(ls->getContainer()->getSocketOnContainer(getStart()) == ls->getContainer()->getInSockets().at(2)) {
//            data = LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getStep();
//        }
//        else{
//            cache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
//        }
//    }
//}
//
//void FloatCache::math(eMathOp op)    
//{
//    DinSocketList insockets = getNode()->getInSockets();
//    int i=0;
//    foreach(DinSocket *socket, insockets){
//        if(i == 0) {
//            int tmpdata = cache(socket);
//        }
//        else {
//            if(!socket->getCntdSocket() && socket->getType() == VARIABLE)
//                continue;
//
//            double tmpdata;
//            cache(socket);
//            switch(op)
//            {
//                case OPADD:
//                    tmpdata += data.getData<double>();
//                    break;
//                case OPSUBTRACT:
//                    tmpdata -= data.getData<double>();
//                    break;
//                case OPMULTIPLY:
//                    tmpdata *= data.getData<double>();
//                    break;
//                case OPDIVIDE:
//                    tmpdata /= data.getData<double>();
//                    break;
//            }
//        }
//        i++;
//    }
//}
//
//void FloatCache::modulo()
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
//void FloatCache::container()    
//{
//    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
//    cache(node->getSocketInContainer(getStart())->toIn());
//}
//
//void FloatCache::stepup()    
//{
//    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
//    if(node->getNodeType() == FOREACHNODE
//        &&getStart() == getStart()->getNode()->getOutSocketLlist()->getLLsocketAt(0)->socket) {
//        data = ((VectorForeachCacheThread*)QThread::currentThread())->getStep();
//        return;
//    }
//    cache(node->getSocketOnContainer(getStart())->toIn());
//}
//
