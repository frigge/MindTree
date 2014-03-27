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

#include "iostream"

#include "cache_main.h"

using namespace MindTree;

std::unordered_map<DataCache*, const DoutSocket*>CacheControl::caches;
std::unordered_map<const LoopNode*, LoopCache*> LoopCacheControl::loops;
ConstNodeList CacheControl::updateNodes;
std::vector<AbstractCacheProcessor::cacheList> DataCache::processors;

bool CacheControl::isCached(const DoutSocket *socket)    
{
    return getCache(socket) ? true : false;
}

void CacheControl::addCache(const DoutSocket *socket, DataCache *cache)    
{
    if(isCached(socket)) {
        DataCache *c = getCache(socket);
        caches.erase(c);
        delete c;
    }
    caches.insert({cache, socket}); 
}

void CacheControl::removeCache(DataCache *cache)    
{
    caches.erase(cache);
}

DataCache* CacheControl::getCache(const DoutSocket *socket)    
{
    for (auto p : caches)
        if (p.second == socket) return p.first;
    return nullptr;
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
    updateNodes.push_back(changedNode);

    for(const DNode *node : active_network) 
    {
        auto b = before_changed.begin();
        auto e = before_changed.end();
        if(std::find(b, e, node) == e)
            updateNodes.push_back(node);
    }
}

bool CacheControl::isOutDated(const DNode *node)    
{
    return std::find(updateNodes.begin(), 
                     updateNodes.end(), 
                     node) != updateNodes.end();
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
    for (auto p : cachedData)
        delete p.first;
    cachedData.clear();
}

void LoopCache::setStep(int step)    
{
    stepValue = step;
}

int LoopCache::getStep()const
{
    return stepValue;
}


/*
 * the cached data is preserved for every loop iteration
 *
 */
void LoopCache::addData(DataCache *cache)    
{
    if(cachedData.size() > 0)
        loopentry = cache->getStart();
    auto p = cachedData.find(cache->getStart());
    if(p != cachedData.end())
    {
        delete p->second;
        cachedData.erase(p);
    }

    cachedData.insert({cache->getStart(), cache});
}

DataCache* LoopCache::getCache(const DoutSocket *socket)    
{
    const DoutSocket *s;
    if(!socket)
        s = loopentry;
    else
        s = socket;

    if(cachedData.find(s) == cachedData.end())
        return 0;
    return cachedData[s];
}

const LoopNode* LoopCache::getNode()
{
    return loopentry->getNode()->getDerivedConst<LoopSocketNode>()->getContainer()->getDerivedConst<LoopNode>();
}

LoopCache* LoopCacheControl::loop(const LoopNode *node)
{
    //QMutex mutex;
    //QMutexLocker lock(&mutex);
    LoopCache *lc;
    if(loops.find(node) != loops.end())
        lc = loops[node];
    else {
        lc = new LoopCache();
        loops.insert({node, lc});
    }
    return lc;
}

void LoopCacheControl::del(const LoopNode *node)    
{
    auto p = loops.find(node);
    loops.erase(p);

    delete p->second;
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
    std::cout << "calling c++ processor ..." << std::endl;
    processor(cache); 
}

DataCache::DataCache()
    : node(nullptr),
    typeID(-1),
    startsocket(nullptr)
{
}

DataCache::DataCache(const DoutSocket *socket)
    : node(socket->getNode()),
    typeID(socket->getType().id()),
    startsocket(socket)
{
    cacheInputs();
}

DataCache::DataCache(const DataCache &other)
    : 
    node(other.node), 
    typeID(other.typeID),
    cachedData(other.cachedData),
    startsocket(other.startsocket)
{
}

DataCache::~DataCache()
{
    std::cout << "delete datacache" << std::endl;
}

void DataCache::start(const DoutSocket *socket)
{
    startsocket = socket;
    node = socket->getNode();
    typeID = socket->getType().id();
    cacheInputs();
}

int DataCache::getTypeID() const
{
    return  typeID;
}

void DataCache::addProcessor(SocketType st, NodeType nt, AbstractCacheProcessor *proc)
{
    unsigned long node_type = nt.id();
    unsigned long socket_type = st.id();
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
       //const DoutSocket *lastSocket = startsocket;
       //const DNode *lastNode = node;
       //startsocket = socket->getCntdSocket();
       //node = socket->getCntdSocket()->getNode();

       //cacheInputs();
       DataCache cache(socket->getCntdSocket());
       pushData(cache.getData(0));

       //restore previouse data when finished
       //startsocket = lastSocket;
       //node = lastNode;
   } 
   else
       pushData(socket->getProperty());
}

void DataCache::pushData(Property prop)
{
    cachedData.push_front(prop);
}

const Property& DataCache::getData(int index) const
{
    return *std::next(cachedData.begin(), index);
}

const DNode* DataCache::getNode() const
{
    return node;
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
    auto insockets = node->getInSockets();
    for(auto it = insockets.rbegin(); it != insockets.rend(); it++) {
        cache(*it);
    }

    unsigned long nodeTypeID = node->getType().id();

    if(typeID >= processors.size()){
        std::cout<< "no processors defined for this data type ("
                 << SocketType::byID(typeID).toStr()
                 << " id:" 
                 << typeID
                 << ")"
                 << std::endl;
        return;
    }
    auto list = processors[typeID];
    if(nodeTypeID >= list.size()){
        std::cout<< "no processors defined for this node type (" 
                 << node->getType().toStr() 
                 << " id:" 
                 <<nodeTypeID
                 <<")"
                 << std::endl;
        return;
    }
    auto datacache = list[nodeTypeID];
    if(!datacache) {
        std::cout<<"Node Type ID:" << nodeTypeID << std::endl;
        std::cout<<"Socket Type ID:" << typeID << std::endl;
        return;
    }
    std::cout << "caching " << SocketType::byID(typeID).toStr() << " on " << node->getType().toStr() << std::endl;
    (*datacache)(this);
    //Property prop(cachedData.front());
    //cachedData.pop_front();
    //for(size_t i = 0; i<node->getInSockets().size(); i++)
    //    cachedData.pop_front();
    //cachedData.push_front(prop);
}
