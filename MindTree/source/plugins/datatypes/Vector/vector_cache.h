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

#ifndef VECTOR_CACHE_REFTCZQO

#define VECTOR_CACHE_REFTCZQO

#include "QThread"
#include "data/cache_main.h"

class VectorList;
namespace MindTree
{
class DinSocket;

class VectorForeachCacheThread : public QThread
{
public:
    VectorForeachCacheThread(const MindTree::DinSocket *socket, VectorList *array, int work_start, int work);
    ~VectorForeachCacheThread();
//    int getStep();
//
//protected:
//    void run();
//
private:
    const MindTree::DinSocket *socket;
    VectorList *array;
    int work_start, work, step;
};

class VectorCache : public DataCache
{
public:
    VectorCache(const MindTree::DoutSocket *socket=0);
    ~VectorCache();
//    virtual VectorCache* getDerived();
//    void clear();
//
//protected:
//    void composeArray();
//    void vectorValue();
//    void floattovector();
//    void setArray();
//    void forloop();
//    void getLoopedCache();
//    void container();
//    void stepup();
//    void foreachloop();
//
//private:
//    Vector data;
};

class VectorCacheThreaded : public QThread
{
public:
    VectorCacheThreaded(const MindTree::DinSocket *socket=0);
    VectorCacheThreaded(const MindTree::DAInSocket *socket=0);
//    VectorList* getData();

protected:
 //   void run();

private:
    const MindTree::DinSocket *socket;
    const MindTree::DAInSocket *asocket;
    VectorList *data;
};

} /* MindTree */


#endif /* end of include guard: VECTOR_CACHE_REFTCZQO */
