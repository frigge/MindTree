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

#include "data/nodes/data_node.h"
#include "QThread"

namespace MindTree
{
    
class DataCache;

class CacheControl
{
public:
    static DataCache *cache(const MindTree::DoutSocket *socket);
    static bool isCached(const MindTree::DoutSocket *socket);
    static void addCache(const MindTree::DoutSocket *socket, DataCache *cache);
    static DataCache* getCache(const MindTree::DoutSocket *socket);
    static void removeCache(DataCache *cache);
    static void analyse(MindTree::DNode *viewnode, const MindTree::DNode *changedNode);
    static bool isOutDated(const MindTree::DNode *node);

private:
    static QHash<DataCache*, const MindTree::DoutSocket*>caches;
    static QList<const MindTree::DNode*> updateNodes;
};

class LoopCache
{
public:
    LoopCache();
    ~LoopCache();
    void setStep(int step);
    int getStep()const;
    void addData(DataCache *cache);
    DataCache* getCache(const MindTree::DoutSocket *socket=0);
    const MindTree::LoopNode* getNode();
    void free();

private:
    const MindTree::DoutSocket *loopentry;
    QHash<const MindTree::DoutSocket*, DataCache*> cachedData;
    int stepValue, startValue, endValue;
};

class LoopCacheControl
{
public:
    static LoopCache* loop(const MindTree::LoopNode *node);
    static void del(const MindTree::LoopNode *node);

private:
    static QHash<const MindTree::LoopNode*, LoopCache*> loops;
};

class AbstractCacheProcessor
{
public:
    typedef std::vector<AbstractCacheProcessor*> cacheList;
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

class DoutSocket;

template<typename T>
struct CacheMap {
    static const std::string str;
    typedef int Type;
    static void factory(const DoutSocket* socket){}
};

template<typename T>
const std::string CacheMap<T>::str="unknown";

#ifndef regCacheType
#define regCacheType(cachetype, datatype)\
template<>\
struct CacheMap<datatype>{\
    typedef cachetype Type;\
    static datatype factory(const DoutSocket* socket){\
        return std::make_shared<cachetype>(socket)->data\
        .getData<datatype>()\
        ;}\
    static const std::string str;\
};\
const std::string CacheMap<datatype>::str = #datatype;\
template<>\
struct CacheMap<cachetype>{\
    typedef datatype Type;\
    static const std::string str;\
};\
const std::string CacheMap<cachetype>::str = #cachetype;
#endif

class DataCache
{
public:
    enum cacheType {
        FLOATCACHE, INTEGERCACHE, SCENECACHE, 
        OBJECTCACHE, STRINGCACHE, BOOLCACHE, 
        VECTORCACHE, COLORCACHE, POLYGONCACHE,
        CAMERACACHE, LIGHTCACHE, OBJECTDATACACHE
    };
    typedef enum eMathOp {
        OPADD, OPSUBTRACT, OPMULTIPLY, OPDIVIDE
    } eMathOp;
    DataCache(const MindTree::DoutSocket *start);
    DataCache(const DataCache &other);
    virtual ~DataCache();
    void cache(const DinSocket *socket);

    int getTypeID();

    template<typename T>
    T cacheForeign(const DinSocket *socket) {
        if(socket->getCntdSocket()){
            startsocket = socket->getCntdSocket();
            return CacheMap<T>::factory(startsocket);
        } 
        else
            return socket->getProperty().getData<T>();
    }

    void setOwner(bool owner)const;
    bool isOwner()const;
    virtual DataCache* getDerived();
    void cacheInputs();
    const MindTree::DoutSocket *getStart()const;
    void setStart(const DoutSocket *socket);
    cacheType getType();
    void setType(DataCache::cacheType value);
    static void addProcessor(SocketType st, NodeType nt, AbstractCacheProcessor *proc);

    DNode* getNode();
    Property data;

    static const std::vector<AbstractCacheProcessor::cacheList>& getProcessors();

protected:
    virtual void container();
    virtual void setArray();
    virtual void composeArray();
    virtual void composePolygon();
    virtual void composeObject();
    virtual void vectorValue();
    virtual void floattovector();
    virtual void intValue();
    virtual void floatValue();
    virtual void forloop();
    virtual void getLoopedCache();
    virtual void math(eMathOp op);
    virtual void modulo();
    virtual void stepup();
    virtual void glShader();
    virtual void transform();
    virtual void foreachloop();
    virtual void createLight();
    virtual void createCamera();
    virtual void composeScene();
    virtual void createMesh();
    virtual void loadObj();
    virtual void composeGroup();


private:
    DNode *node;
    static std::vector<AbstractCacheProcessor::cacheList> processors;
    int typeID;
    cacheType type;
    const DoutSocket *startsocket;
    mutable bool dataOwner;
};


} /* MindTree */

#endif /* end of include guard: CACHE_MAIN_PD1QWTW9 */
