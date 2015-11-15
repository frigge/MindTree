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

#include "data/dnspace.h"
#include "data/nodes/containernode.h"
#include "data/signal.h"
#include "data/debuglog.h"

#include "cache_main.h"

using namespace MindTree;

TypeDispatcher<SocketType, AbstractCacheProcessor::CacheList> DataCache::processors;
AbstractCacheProcessor::CacheList DataCache::_genericProcessors;

CacheContext::CacheContext(const LoopNode* node)
    : _node(node)
{
}

CacheContext::~CacheContext()
{
}

/*
 * the cached data is preserved for every loop iteration
 *
 */
void CacheContext::addData(size_t index, Property prop)
{
    if (_data.size() <= index)
        _data.resize(index + 1);

    _data[index] = prop;
}

Property CacheContext::getData(size_t i)
{
    //find index
    if(i < _data.size())
        return _data[i];
    return Property();
}

Property CacheContext::getData(const DoutSocket *socket)
{
    //find index
    auto outsockets = getNode()->getLoopedInputs()->getOutSockets();
    auto outiter = std::find(begin(outsockets), end(outsockets), socket);
    if (outiter == end(outsockets))
        return Property();

    size_t i = std::distance(outiter, begin(outsockets));
    return getData(i);
}

const LoopNode* CacheContext::getNode()
{
    return _node;
}

LoopCache::LoopCache(const MindTree::LoopNode *node)
    : CacheContext(node), stepValue(0), startValue(0), endValue(0)
{
}

LoopCache::~LoopCache()
{
}

void LoopCache::setStep(int step)
{
    stepValue = step;
    auto nodes = getNode()->getContainerData()->getNodes();
    DataCache::invalidate(nodes[0].get()); //static inputs
    DataCache::invalidate(nodes[1].get()); //looped inputs
}

int LoopCache::getStep()const
{
    return stepValue;
}

AbstractCacheProcessor::AbstractCacheProcessor(SocketType st, NodeType nt) :
    m_socketType(st), m_nodeType(nt)
{
}

AbstractCacheProcessor::~AbstractCacheProcessor()
{
}

const SocketType& AbstractCacheProcessor::getSocketType() const
{
    return m_socketType;
}

const NodeType& AbstractCacheProcessor::getNodeType() const
{
    return m_nodeType;
}

CacheProcessor::CacheProcessor(SocketType st, NodeType nt, std::function<void(DataCache*)> fn)
    : AbstractCacheProcessor(st, nt), processor(fn)
{
}

CacheProcessor::~CacheProcessor()
{
}

void CacheProcessor::operator()(DataCache* cache)
{
    processor(cache); 
}

std::unordered_map<const DNode*, std::vector<Property>> DataCache::_cachedOutputs;
std::recursive_mutex DataCache::_cachedOutputsMutex;
std::mutex DataCache::_processorMutex;
DataCache::DataCache(CacheContext *context)
    : node(nullptr),
    startsocket(nullptr),
    _context(context)
{
}

DataCache::DataCache(const DNode *node, DataType t, CacheContext *context)
    : node(node), type(t), _context(context)
{
    cacheInputs();
}

DataCache::DataCache(const DoutSocket *socket, CacheContext *context)
    : node(socket->getNode()),
    type(socket->getType()),
    startsocket(socket),
    _context(context)
{
    cacheInputs();
}

DataCache::DataCache(const DataCache &other)
    : 
    node(other.node), 
    cachedInputs(other.cachedInputs),
    type(other.type),
    startsocket(other.startsocket),
    _context(other._context)
{
}

DataCache::~DataCache()
{
}

void DataCache::init()
{
    Signal::getHandler<DNode*>().connect("nodeDeleted", [] (DNode* node) {
        DataCache::invalidate(node);
    }).detach();
}

CacheContext* DataCache::getContext()
{
    return _context;
}

void DataCache::setContext(CacheContext *context)
{
    _context = context;
}

void DataCache::invalidateNode(const DNode *node)
{
    if(!node) return;

    {
        std::lock_guard<std::recursive_mutex> lock(_cachedOutputsMutex);
        auto cacheIter = _cachedOutputs.find(node);
        if(cacheIter == end(_cachedOutputs))
            return;

        _cachedOutputs.erase(cacheIter);
    }
}

void DataCache::invalidate(const DNode *node)
{
    invalidateNode(node);

    //recursively invalidate everything following this node
    for(const auto *out : node->getOutSockets()) {
        for(const auto *ins : out->getCntdSockets()) {
            DNode* n = ins->getNode();
            if(n->getBuildInType() == DNode::SOCKETNODE) {
                ContainerNode *cnode = dynamic_cast<SocketNode*>(n)->getContainer();
                invalidateNode(cnode);
                for(auto cout : cnode->getOutSockets()) {
                    for(auto *cntdOnCont : cout->getCntdSockets()) {
                        invalidate(cntdOnCont->getNode());
                    }
                }
            } else {
                invalidate(ins->getNode());
            }
        }
    }

    if(node->getBuildInType() == DNode::CONTAINER) {
        const auto *contnode = node->getDerivedConst<ContainerNode>();
        auto *contData = contnode->getContainerData();
        if(contData)
            for (const auto n : contnode->getContainerData()->getNodes())
                invalidate(n.get());
    }

}

bool DataCache::isCached(const DNode *node)
{
    std::lock_guard<std::recursive_mutex> lock(_cachedOutputsMutex);
    bool cached = (_cachedOutputs.find(node) != end(_cachedOutputs))
        && (!_cachedOutputs[node].empty());
    return cached;
}

void DataCache::start(const DoutSocket *socket)
{
    startsocket = socket;
    cachedInputs.clear();
    if(socket) {
        node = socket->getNode();
        type = socket->getType();
    }
    cacheInputs();
}

int DataCache::getTypeID() const
{
    return  type.id();
}

std::vector<Property>& DataCache::_getCachedOutputs()
{
    return getCachedOutputs(node);
}

std::vector<Property>& DataCache::getCachedOutputs(const DNode *node)
{
    std::lock_guard<std::recursive_mutex> lock(_cachedOutputsMutex);
    if(_cachedOutputs.find(node) == end(_cachedOutputs))
        _cachedOutputs.insert({node, std::vector<Property>()});
    return _cachedOutputs[node];
}

DataType DataCache::getType() const
{
    return type;
}

void DataCache::addProcessor(AbstractCacheProcessor *proc)
{
    auto st = proc->getSocketType();
    auto nt = proc->getNodeType();
    std::lock_guard<std::mutex> lock(_processorMutex);
    processors[st][nt].reset(proc);
}

void DataCache::removeProcessor(AbstractCacheProcessor *proc)
{
    std::lock_guard<std::mutex> lock(_processorMutex);
    processors[proc->getSocketType()][proc->getNodeType()] = std::shared_ptr<AbstractCacheProcessor>();
}

void DataCache::addGenericProcessor(GenericCacheProcessor *proc)
{
    _genericProcessors[proc->getNodeType()].reset(proc);
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
    auto insockets = node->getInSockets();

    int index = std::distance(begin(insockets),
                              std::find(begin(insockets),
                                        end(insockets),
                                        socket));
   if(socket->getCntdSocket()){
       DoutSocket *out = socket->getCntdSocket();
       DataCache cache(out, _context);
       _pushInputData(cache.getOutput(out), index);
   } 
   else
       _pushInputData(socket->getProperty(), index);
}

void DataCache::_pushInputData(Property prop, int index)
{
    size_t i = index;
    if(index < 0 || i == cachedInputs.size()) {
        cachedInputs.push_back(prop);
        return;
    }

    if(static_cast<size_t>(index) >= cachedInputs.size())
        cachedInputs.resize(index + 1);
    cachedInputs[index] = prop;
}

void DataCache::pushData(Property prop, int index)
{
    std::lock_guard<std::recursive_mutex> lock(_cachedOutputsMutex);
    auto &outputs = _getCachedOutputs();

    size_t i = index;
    if (index < 0 || i == outputs.size()) {
        outputs.push_back(prop);
        return;
    }

    if(static_cast<size_t>(index) < outputs.size()) {
        outputs.resize(index + 1); 
    }

    outputs[index] = prop;
}

//returns the value of the input socket at index i
Property DataCache::getData(int index)
{
    size_t i = index;
    auto insockets = node->getInSockets();

    if (i >= insockets.size()) 
        return Property();

    cache(insockets.at(index));

    if (i >= cachedInputs.size()) 
        return Property();

    return cachedInputs[index];
}

Property DataCache::getOutput(int index)
{
    std::lock_guard<std::recursive_mutex> lock(_cachedOutputsMutex);
    auto &outputs = _getCachedOutputs();
    size_t i = index;
    if(i >= outputs.size())
        return Property();

    return outputs[index];
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
    return getOutput(index);
}

void DataCache::setNode(const DNode *n)
{
    node = n;
}

void DataCache::setType(DataType t)
{
    type = t;
}

const DNode* DataCache::getNode() const
{
    return node;
}

const DoutSocket* DataCache::getStart() const
{
    return startsocket;
}

void DataCache::setStart(const DoutSocket *socket)
{
    startsocket = socket;
}

void DataCache::cacheInputs()
{
    if(isCached(node))
        return;

    auto ntype = node->getType();
    unsigned long nodeTypeID = ntype.id();
    std::string nodeName = node->getNodeName();

    auto &genericProcessor = _genericProcessors[ntype];
    if(genericProcessor) {
        (*genericProcessor)(this);
        return;
    }

    if(type.id() >= static_cast<int>(processors.size())){
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
                 << " on node: "
                 << nodeName
                 << std::endl;
        return;
    }
    auto &datacache = list[node->getType()];
    if(!datacache) {
        std::cout<<"Node Type ID:" << nodeTypeID << std::endl;
        std::cout<<"Socket Type ID:" << type.id() << std::endl;
        return;
    }
    (*datacache)(this);
    std::string status = "done caching caching: " + node->getNodeName();
    MT_SIGNAL_EMITTER("STATUSUPDATE", status);
    dbout(status);

}
