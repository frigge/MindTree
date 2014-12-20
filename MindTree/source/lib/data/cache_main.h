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

#ifndef CACHE_MAIN_PD1QWTW9

#define CACHE_MAIN_PD1QWTW9

#include "mutex"
#include "data/type.h"
#include "data/nodes/data_node_socket.h"
#include "data/nodes/containernode.h"

namespace MindTree
{
class Property;
    
class DataCache;

class CacheContext
{
public:
    CacheContext(const LoopNode *node);
    virtual ~CacheContext();

    void addData(size_t index, Property prop);
    virtual Property getData(const MindTree::DoutSocket *socket=0);
    Property getData(size_t i);
    virtual const LoopNode* getNode();

private:
    const LoopNode *_node;
    std::vector<Property> _data;

};

class LoopCache : public CacheContext
{
public:
    LoopCache(const LoopNode *node);
    ~LoopCache();

    void setStep(int step);
    int getStep()const;

private:
    int stepValue, startValue, endValue;
};

class AbstractCacheProcessor
{
public:
    typedef TypeDispatcher<NodeType, AbstractCacheProcessor*> CacheList;
    AbstractCacheProcessor();
    virtual ~AbstractCacheProcessor();
    virtual void operator()(DataCache*)=0;
};

class CacheProcessor : public AbstractCacheProcessor
{
public:
    CacheProcessor(std::function<void(DataCache*)> fn);
    virtual ~CacheProcessor();
    void operator()(DataCache* cache);

private:
    std::function<void(DataCache*)> processor;
};

class DataCache
{
public:
    DataCache(CacheContext *context = nullptr);
    DataCache(const MindTree::DoutSocket *start, CacheContext *context = nullptr);
    DataCache(const DNode *node, DataType t, CacheContext *context = nullptr);

    DataCache(const DataCache &other);
    virtual ~DataCache();

    static void init();

    void start(const DoutSocket *socket=nullptr);

    int getTypeID() const;
    DataType getType() const;

    void pushData(Property prop, int index = -1);
    Property getData(int index);
    Property getOutput(DoutSocket* socket = nullptr);
    Property getOutput(int index);
    void setNode(const DNode *n);
    void setType(DataType t);

    const DNode* getNode()const;

    const MindTree::DoutSocket *getStart()const;
    void setStart(const DoutSocket *socket);

    static void addProcessor(SocketType st, NodeType nt, AbstractCacheProcessor *proc);
    static void addGenericProcessor(NodeType nt, AbstractCacheProcessor *proc);
    static const std::vector<AbstractCacheProcessor::CacheList>& getProcessors();
    static std::vector<Property>& getCachedOutputs(const DNode *node);
    static void invalidate(const DNode *node);
    static bool isCached(const DNode *node);

    CacheContext* getContext();
    void setContext(CacheContext *context);

private:
    std::vector<Property>& _getCachedOutputs();
    void _pushInputData(Property prop, int index = -1);

    void cacheInputs();
    void cache(const DinSocket *socket);

    const DNode *node;
    static TypeDispatcher<SocketType, AbstractCacheProcessor::CacheList> processors;
    static AbstractCacheProcessor::CacheList _genericProcessors;
    std::vector<Property> cachedInputs;
    SocketType type;
    const DoutSocket *startsocket;
    static std::unordered_map<const DNode*, std::vector<Property>> _cachedOutputs;
    static std::recursive_mutex _cachedOutputsMutex;

    CacheContext *_context;
};


} /* MindTree */

#endif /* end of include guard: CACHE_MAIN_PD1QWTW9 */
