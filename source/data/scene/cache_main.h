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
    static bool isCached(DoutSocket *socket);
    static void addCache(DoutSocket *socket, AbstractDataCache *cache);
    static void removeCache(AbstractDataCache *cache);

private:
    static QHash<AbstractDataCache*, DoutSocket*>caches;
};

class AbstractDataCache
{
public:
    AbstractDataCache(DoutSocket *start);
    void cacheSocket(DoutSocket *socket);
    virtual AbstractDataCache* getDerived();
    void cacheInputs();

protected:
    DoutSocket *getStart();
    virtual void composeArray();
    virtual void composePolygon();
    virtual void composeObject();
    virtual void vectorValue();
    virtual void intValue();

private:
    DoutSocket *start;
    DSocketList cachedSockets;
};


#endif /* end of include guard: CACHE_MAIN_PD1QWTW9 */
