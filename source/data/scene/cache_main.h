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

class AbstractDataCache;

class CacheControl
{
public:
    static AbstractDataCache *cache(const DoutSocket *socket);
    static bool isCached(const DoutSocket *socket);
    static void addCache(const DoutSocket *socket, AbstractDataCache *cache);
    static void removeCache(AbstractDataCache *cache);

private:
    static QHash<AbstractDataCache*, const DoutSocket*>caches;
};

class LoopCache
{
public:
    LoopCache();
    ~LoopCache();
    void setStep(int step);
    int getStep();
    void addData(AbstractDataCache *cache);
    AbstractDataCache* getCache(const DoutSocket *socket=0);
    const LoopNode* getNode();
    void free();

private:
    const DoutSocket *loopentry;
    QHash<const DoutSocket*, AbstractDataCache*> cachedData;
    int stepValue;
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
    AbstractDataCache(const DinSocket *start);
    AbstractDataCache(const AbstractDataCache &cache);
    virtual ~AbstractDataCache();
    void setOwner(bool owner)const;
    void cacheSocket(DoutSocket *socket);
    virtual AbstractDataCache* getDerived();
    void cacheInputs();
    const DoutSocket *getStart();

protected:
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
    virtual void add();
    virtual void subtract();
    virtual void multiply();
    virtual void divide();

private:
    const DoutSocket *start;
    const DinSocket *inSocket;
    DSocketList cachedSockets;
    mutable bool dataOwner;
};


#endif /* end of include guard: CACHE_MAIN_PD1QWTW9 */
