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

#ifndef PYCACHE_MAIN_G6RPVCHD

#define PYCACHE_MAIN_G6RPVCHD

#include "data/cache_main.h"
#include "wrapper.h"

namespace MindTree
{
void wrap_DataCache();
DNodePyWrapper* wrap_DataCache_getNode(DataCache *cache);
DoutSocketPyWrapper* wrap_DataCache_getStart(DataCache *cache);
void wrap_DataCache_addProcessor(BPy::object fn, std::string ntype, std::string stype);
BPy::object wrap_DataCache_getData(DataCache *self, int index);
BPy::object wrap_DataCache_getOutput(MindTree::DataCache *self, DoutSocketPyWrapper *outsocket);
BPy::object wrap_DataCache_getOutput(MindTree::DataCache *self);
void wrap_DataCache_setData(DataCache *self, BPy::object data);
void wrap_DataCache_cache(MindTree::DataCache *self, MindTree::DinSocketPyWrapper* socket);
BPy::dict wrap_DataCache_getProcessors();
std::string wrap_DataCache_getType(DataCache *self);

//template<typename T>
//class PyWrapCache : public T
//{
//    typedef typename CacheMap<T>::Type datatype_t;
//public:
//    PyWrapCache(DoutSocketPyWrapper* socket) : T(socket->getWrapped<DoutSocket>()) {}
//    virtual ~PyWrapCache(){}
//    static void wrap(){
//        BPy::class_<PyWrapCache<T>, BPy::bases<DataCache>>(CacheMap<T>::str.c_str(), BPy::init<DoutSocketPyWrapper*>());
//    }
//};

class PyWrapCache : public DataCache
{
public:
    PyWrapCache(DoutSocketPyWrapper* socket) : DataCache(socket->getWrapped<DoutSocket>()) {}
    virtual ~PyWrapCache(){}
};

class DinSocketPyWrapper; 

class PyCacheProcessor : public AbstractCacheProcessor
{
public:
    PyCacheProcessor(BPy::object);
    virtual ~PyCacheProcessor();
    void operator()(DataCache* cache);

private:
    BPy::object processor;
};


} /* MindTree */

#endif /* end of include guard: PYCACHE_MAIN_G6RPVCHD */
