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

#include "cache_main.h"
#include "source/data/nodes/data_node.h"
#include "QThread"

class AbstractDataCache;
class ViewportNode;

class CacheControl
{
public:
    static AbstractDataCache *cache(const DoutSocket *socket);
    static bool isCached(const DoutSocket *socket);
    static void addCache(const DoutSocket *socket, AbstractDataCache *cache);
    static AbstractDataCache* getCache(const DoutSocket *socket);
    static void removeCache(AbstractDataCache *cache);
    static void analyse(ViewportNode *viewnode, DNode *changedNode);
    static bool isOutDated(DNode *node);

private:
    static QHash<AbstractDataCache*, const DoutSocket*>caches;
    static QList<DNode*> updateNodes;
};

class LoopCache
{
public:
    LoopCache();
    ~LoopCache();
    void setStep(int step);
    int getStep()const;
    void addData(AbstractDataCache *cache);
    AbstractDataCache* getCache(const DoutSocket *socket=0);
    const LoopNode* getNode();
    void free();

private:
    const DoutSocket *loopentry;
    QHash<const DoutSocket*, AbstractDataCache*> cachedData;
    int stepValue, startValue, endValue;
};

class LoopCacheControl
{
public:
    static LoopCache* loop(const LoopNode *node);
    static void del(const LoopNode *node);

private:
    static QHash<const LoopNode*, LoopCache*> loops;
};

class AbstractDataCache
{
public:
    enum cacheType {
        FLOATCACHE, INTEGERCACHE, SCENECACHE, STRINGCACHE, BOOLCACHE, VECTORCACHE, COLORCACHE, POLYGONCACHE
    };
    typedef enum eMathOp {
        OPADD, OPSUBTRACT, OPMULTIPLY, OPDIVIDE
    } eMathOp;
    AbstractDataCache(const DinSocket *start);
    AbstractDataCache(const AbstractDataCache &cache);
    virtual ~AbstractDataCache();
    void setOwner(bool owner)const;
    bool isOwner()const;
    virtual AbstractDataCache* getDerived();
    void cacheInputs();
    const DoutSocket *getStart()const;
    cacheType getType();
    void setType(AbstractDataCache::cacheType value);
    void update();

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

private:
    cacheType type;
    const DoutSocket *startsocket;
    const DinSocket *inSocket;
    mutable bool dataOwner;
};


#endif /* end of include guard: CACHE_MAIN_PD1QWTW9 */
