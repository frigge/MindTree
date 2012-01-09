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

#include "cache_main.h"
#include "QMutex"
#include "QMutexLocker"

#include "source/graphics/viewport.h"

QHash<AbstractDataCache*, const DoutSocket*>CacheControl::caches;
QHash<const LoopNode*, LoopCache*> LoopCacheControl::loops;
QList<DNode*> CacheControl::updateNodes;

bool CacheControl::isCached(const DoutSocket *socket)    
{
    return caches.values().contains(socket);
}

void CacheControl::addCache(const DoutSocket *socket, AbstractDataCache *cache)    
{
    if(caches.contains(caches.key(socket))) {
        delete caches.key(socket);
        caches.remove(caches.key(socket));
    }
    caches.insert(cache, socket); 
}

void CacheControl::removeCache(AbstractDataCache *cache)    
{
    caches.remove(cache);
}

AbstractDataCache* CacheControl::getCache(const DoutSocket *socket)    
{
    return caches.key(socket); 
}

void CacheControl::analyse(ViewportNode *viewnode, DNode *changedNode)    
{
    updateNodes.clear();
    NodeList active_network = viewnode->getAllInNodes(); 
    if(!changedNode){
        updateNodes = active_network;
        return;
    }
    NodeList before_changed = changedNode->getAllInNodes();
    updateNodes.append(changedNode);

    foreach(DNode *node, active_network) 
        if(!before_changed.contains(node))
            updateNodes.append(node);
}

bool CacheControl::isOutDated(DNode *node)    
{
    return updateNodes.contains(node);
}

LoopCache::LoopCache()
    : loopentry(0), stepValue(0), startValue(0), endValue(0)
{
}

LoopCache::~LoopCache()
{
    free();
}

void LoopCache::free()    
{
    while(!cachedData.isEmpty())
        foreach(const DoutSocket *socket, cachedData.keys())
            delete cachedData.take(socket);
}

void LoopCache::setStep(int step)    
{
    stepValue = step;
}

int LoopCache::getStep()const
{
    return stepValue;
}


void LoopCache::addData(AbstractDataCache *cache)    
{
    if(cachedData.isEmpty())
        loopentry = cache->getStart();
    if(cachedData.contains(cache->getStart()))
        delete cachedData.take(cache->getStart());

    cachedData.insert(cache->getStart(), cache);
}

AbstractDataCache* LoopCache::getCache(const DoutSocket *socket)    
{
    const DoutSocket *s;
    if(!socket)
        s = loopentry;
    else
        s = socket;

    if(!cachedData.contains(s))
        return 0;
    return cachedData.value(s);
}

const LoopNode* LoopCache::getNode()
{
    return loopentry->getNode()->getDerivedConst<LoopSocketNode>()->getContainer()->getDerivedConst<LoopNode>();
}

LoopCache* LoopCacheControl::loop(const LoopNode *node)
{
    QMutex mutex;
    QMutexLocker lock(&mutex);
    LoopCache *lc;
    if(loops.contains(node))
        lc = loops.value(node);
    else {
        lc = new LoopCache();
        loops.insert(node, lc);
    }
    return lc;
}

void LoopCacheControl::del(const LoopNode *node)    
{
    QMutex mutex;
    QMutexLocker lock(&mutex);
    delete loops.take(node);
}

AbstractDataCache::AbstractDataCache(const DinSocket *socket)
    : startsocket(socket->getCntdSocket()), inSocket(socket), dataOwner(true)
{
}

AbstractDataCache::~AbstractDataCache()
{
}

void AbstractDataCache::update()    
{
    startsocket = inSocket->getCntdSocket();
    cacheInputs();
}

AbstractDataCache::cacheType AbstractDataCache::getType()
{
    return type;
}

void AbstractDataCache::setType(AbstractDataCache::cacheType value)
{
    type = value;
}

void AbstractDataCache::setOwner(bool owner)const
{
    dataOwner = owner;
}

bool AbstractDataCache::isOwner()    const
{
    return dataOwner;
}

AbstractDataCache* AbstractDataCache::getDerived()    
{
    return this; 
}

const DoutSocket* AbstractDataCache::getStart()    const
{
    return startsocket;
}

void AbstractDataCache::cacheInputs()    
{
    if(!startsocket)
        return;
    switch(startsocket->getNode()->getNodeType())
    {
    case CONTAINER:
        container();
        break;
    case CONDITIONCONTAINER:
        break;
    case FOR:
        forloop();
        break;
    case WHILE:
        break;
    case COLORNODE:
        break;
    case STRINGNODE:
        break;
    case FLOATNODE:
        floatValue();
        break;
    case INTNODE:
        intValue();
        break;
    case VECTORNODE:
        vectorValue();
        break;
    case FLOATTOVECTOR:
        floattovector();
        break;
    case INSOCKETS:
        stepup();
        break;
    case GETARRAY:
        break;
    case SETARRAY:
        setArray();
        break;
    case COMPOSEARRAY:
        composeArray();
        break;
    case OBJECTNODE:
        composeObject();
        break;
    case POLYGONNODE:
        composePolygon();
        break;
    case ADD:
        math(OPADD);
        break;
    case SUBTRACT:
        math(OPSUBTRACT);
        break;
    case MULTIPLY:
        math(OPMULTIPLY);
        break;
    case DIVIDE:
        math(OPDIVIDE);
        break;
    case MODULO:
        modulo();
        break;
    case DOTPRODUCT:
    case GREATERTHAN:
    case SMALLERTHAN:
    case EQUAL:
    case AND:
    case OR:
    case NOT:
    case VARNAME:
        break;
    case LOOPINSOCKETS:
        getLoopedCache();
        break;
    case FRAGMENTOUTPUT:
    case VERTEXOUTPUT:
    case GEOMETRYOUTPUT:
        glShader();
        break;
    case TRANSFORM:
        transform();
        break;
    case FOREACHNODE:
        foreachloop();
        break;
    default:
        break;
            
    }
}

void AbstractDataCache::setArray()    
{
}

void AbstractDataCache::composeArray()    
{
}

void AbstractDataCache::composePolygon()    
{
    
}

void AbstractDataCache::composeObject()    
{
}

void AbstractDataCache::vectorValue()    
{
}

void AbstractDataCache::floattovector()    
{
}

void AbstractDataCache::intValue()    
{
}

void AbstractDataCache::floatValue()    
{
}

void AbstractDataCache::forloop()    
{
}

void AbstractDataCache::getLoopedCache()    
{
}

void AbstractDataCache::math(eMathOp op)    
{
}

void AbstractDataCache::modulo()
{
}

void AbstractDataCache::stepup()    
{
}

void AbstractDataCache::container()    
{
}

void AbstractDataCache::glShader()    
{
}

void AbstractDataCache::transform()    
{
}

void AbstractDataCache::foreachloop()    
{
}
