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
TypeDispatcher<SocketType, AbstractCacheProcessor::CacheList> DataCache::processors;

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
    processor(cache); 
}

DataCache::DataCache()
    : node(nullptr),
    startsocket(nullptr)
{
}

DataCache::DataCache(const DoutSocket *socket)
    : node(socket->getNode()),
    type(socket->getType()),
    startsocket(socket)
{
    cacheInputs();
}

DataCache::DataCache(const DataCache &other)
    : 
    node(other.node), 
    cachedInputs(other.cachedInputs),
    cachedOutputs(other.cachedOutputs),
    type(other.type),
    startsocket(other.startsocket)
{
}

DataCache::~DataCache()
{
}

void DataCache::start(const DoutSocket *socket)
{
    startsocket = socket;
    cachedInputs.clear();
    cachedOutputs.clear();
    node = socket->getNode();
    type = socket->getType();
    cacheInputs();
}

int DataCache::getTypeID() const
{
    return  type.id();
}

DataType DataCache::getType() const
{
    return type;
}

void DataCache::addProcessor(SocketType st, NodeType nt, AbstractCacheProcessor *proc)
{
    processors[st][nt] = proc;
}

const std::vector<AbstractCacheProcessor::CacheList>& DataCache::getProcessors()
{
    return processors.getAll(); 
}

//computes the value of the given input socket
//either by evaluating the connected network or if nothing
//is connected by just taking the property of the input socket
void DataCache::cache(const DinSocket *socket)    
{
   if(socket->getCntdSocket()){
       DoutSocket *out = socket->getCntdSocket();
       DataCache cache(out);
       cachedInputs.push_back(cache.getOutput(out));
   } 
   else
       cachedInputs.push_back(socket->getProperty());
}

void DataCache::pushData(Property prop)
{
    cachedOutputs.push_back(prop);
}

//returns the value of the input socket at index i
Property DataCache::getData(int index)
{
    auto insockets = getNode()->getInSockets();

    if (index >= insockets.size()) 
        return Property();

    cache(insockets.at(index));

    if (index >= cachedInputs.size()) 
        return Property();

    return cachedInputs[index];
}

Property DataCache::getOutput(DoutSocket* socket)
{
    int index = -1;
    if(socket) {
        //find out index of start socket
        int i = 0;

        for(const auto *_socket : node->getOutSockets()) {
            if (_socket == socket) {
                index = i;
                break;
            }
            ++i;
        }
    }
    else {
        index = 0;
    }
    if(index >= cachedOutputs.size())
        return Property();

    return cachedOutputs[index];
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
    unsigned long nodeTypeID = node->getType().id();

    if(type.id() >= processors.size()){
        std::cout<< "no processors defined for this data type ("
                 << type.toStr()
                 << " id:" 
                 << type.id()
                 << ")"
                 << std::endl;
        return;
    }
    auto list = processors[type];
    if(nodeTypeID >= list.size()){
        std::cout<< "no processors defined for this node type (" 
                 << node->getType().toStr() 
                 << " id:" 
                 <<nodeTypeID
                 <<")"
                 << std::endl;
        return;
    }
    auto datacache = list[node->getType()];
    if(!datacache) {
        std::cout<<"Node Type ID:" << nodeTypeID << std::endl;
        std::cout<<"Socket Type ID:" << type.id() << std::endl;
        return;
    }
    (*datacache)(this);
    //Property prop(cachedData.front());
    //cachedData.pop_front();
    //for(size_t i = 0; i<node->getInSockets().size(); i++)
    //    cachedData.pop_front();
    //cachedData.push_front(prop);
}
