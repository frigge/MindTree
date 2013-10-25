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

#include "QMutex"
#include "QMutexLocker"

#include "iostream"

#include "cache_main.h"

using namespace MindTree;

QHash<DataCache*, const DoutSocket*>CacheControl::caches;
QHash<const LoopNode*, LoopCache*> LoopCacheControl::loops;
QList<const DNode*> CacheControl::updateNodes;
std::vector<AbstractCacheProcessor::cacheList> DataCache::processors;

bool CacheControl::isCached(const DoutSocket *socket)    
{
    return caches.values().contains(socket);
}

void CacheControl::addCache(const DoutSocket *socket, DataCache *cache)    
{
    if(caches.contains(caches.key(socket))) {
        delete caches.key(socket);
        caches.remove(caches.key(socket));
    }
    caches.insert(cache, socket); 
}

void CacheControl::removeCache(DataCache *cache)    
{
    caches.remove(cache);
}

DataCache* CacheControl::getCache(const DoutSocket *socket)    
{
    return caches.key(socket); 
}

void CacheControl::analyse(DNode *viewnode, const DNode *changedNode)    
{
    updateNodes.clear();
    ConstNodeList active_network = viewnode->getAllInNodesConst(); 
    if(!changedNode){
        updateNodes = active_network;
        return;
    }
    ConstNodeList before_changed = changedNode->getAllInNodesConst();
    updateNodes.append(changedNode);

    foreach(const DNode *node, active_network) 
        if(!before_changed.contains(node))
            updateNodes.append(node);
}

bool CacheControl::isOutDated(const DNode *node)    
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


void LoopCache::addData(DataCache *cache)    
{
    if(cachedData.isEmpty())
        loopentry = cache->getStart();
    if(cachedData.contains(cache->getStart()))
        delete cachedData.take(cache->getStart());

    cachedData.insert(cache->getStart(), cache);
}

DataCache* LoopCache::getCache(const DoutSocket *socket)    
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

AbstractCacheProcessor::AbstractCacheProcessor()
{
}

AbstractCacheProcessor::~AbstractCacheProcessor()
{
}

CacheProcessor::CacheProcessor(std::function<void(DataCache*)> fn)
    : processor(fn)
{
}

CacheProcessor::~CacheProcessor()
{
}

void CacheProcessor::operator()(DataCache* cache)    
{
    processor(cache); 
}

DataCache::DataCache(const DoutSocket *socket)
    : startsocket(socket), dataOwner(true), node(socket->getNode()),
    typeID(socket->getType().id())
{
    cacheInputs();
}

DataCache::DataCache(const DataCache &other)
    : startsocket(other.startsocket), dataOwner(other.dataOwner),
    node(other.node), typeID(other.typeID), data(other.data)
{
}

DataCache::~DataCache()
{
    std::cout << "delete datacache" << std::endl;
}

int DataCache::getTypeID()
{
    return  typeID;
}

void DataCache::addProcessor(SocketType st, NodeType nt, AbstractCacheProcessor *proc)
{
    int node_type = nt.id();
    int socket_type = st.id();
    if(processors.size() <= socket_type) {
        processors.resize(socket_type+1);
        processors[socket_type] = AbstractCacheProcessor::cacheList();
    }
    if(processors[socket_type].size() <= node_type) {
        processors[socket_type].resize(node_type+1);
    }
    processors[socket_type][node_type] = proc;
}

const std::vector<AbstractCacheProcessor::cacheList>& DataCache::getProcessors()
{
    return processors; 
}

void DataCache::cache(const DinSocket *socket)    
{
   if(socket->getCntdSocket()){
       startsocket = socket->getCntdSocket();
       cacheInputs();
   } 
   else
       data = socket->getProperty();
}

DNode* DataCache::getNode()    
{
    return node;
}

DataCache::cacheType DataCache::getType()
{
    return type;
}

void DataCache::setType(DataCache::cacheType value)
{
    type = value;
}

void DataCache::setOwner(bool owner)const
{
    dataOwner = owner;
}

bool DataCache::isOwner()    const
{
    return dataOwner;
}

DataCache* DataCache::getDerived()    
{
    return this; 
}

const DoutSocket* DataCache::getStart()    const
{
    return startsocket;
}

void DataCache::setStart(const DoutSocket *socket)    
{
    startsocket = socket; 
}

void DataCache::cacheInputs()    
{
    node = startsocket->getNode();
    int nodeTypeID = node->getType().id();
    AbstractCacheProcessor *proc=0;
    if(typeID >= processors.size()){
        std::cout<< "no processors defined for this data type (id:" <<typeID<<")"<< std::endl;
        return;
    }
    auto list = processors[typeID];
    if(nodeTypeID >= list.size()){
        std::cout<< "no processors defined for this node type (id:" <<nodeTypeID<<")"<< std::endl;
        return;
    }
    auto datacache = list[nodeTypeID];
    if(!datacache) {
        std::cout<<"Node Type ID:" << nodeTypeID << std::endl;
        std::cout<<"Socket Type ID:" << typeID << std::endl;
        return;
    }
    (*datacache)(this);
}

void DataCache::setArray()    
{
}

void DataCache::composeArray()    
{
}

void DataCache::composePolygon()    
{
    
}

void DataCache::composeObject()    
{
}

void DataCache::vectorValue()    
{
}

void DataCache::floattovector()    
{
}

void DataCache::intValue()    
{
}

void DataCache::floatValue()    
{
}

void DataCache::forloop()    
{
}

void DataCache::getLoopedCache()    
{
}

void DataCache::math(eMathOp op)    
{
}

void DataCache::modulo()
{
}

void DataCache::stepup()    
{
}

void DataCache::container()    
{
}

void DataCache::glShader()    
{
}

void DataCache::transform()    
{
}

void DataCache::foreachloop()    
{
}

void DataCache::composeScene()    
{
}

void DataCache::createCamera()    
{
}

void DataCache::createLight()    
{
}

void DataCache::createMesh()    
{
}

void DataCache::loadObj()    
{
}

void DataCache::composeGroup()    
{
}
