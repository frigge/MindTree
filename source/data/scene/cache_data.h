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

#ifndef CACHE_DATA_77LKPR3V

#define CACHE_DATA_77LKPR3V

#include "source/data/scene/cache_main.h"
#include "source/data/scene/object.h"

class Object;
class SceneCache : public AbstractDataCache
{
public:
    SceneCache(const DinSocket *node);
    ~SceneCache();
    QList<Object*> getData();
    virtual SceneCache* getDerived();
    void clear();
    Object* newObject(const DNode *node);

protected:
    virtual void composeArray();
    void composeObject();
    void container();
    void stepup();

private:
    QList<Object*> objects;
};

class PolygonCache : public AbstractDataCache
{
public:
    PolygonCache(const DinSocket *socket);
    ~PolygonCache();
    virtual PolygonCache* getDerived();
    PolygonList* getData();
    Polygon getSingleData();
    void clear();

protected:
    void composePolygon();
    void composeArray();
    void container();
    void stepup();
    void setArray();
    void getLoopedCache();
    void forloop();

private:
    PolygonList *data;
    Polygon singlePolygon;
};

class FloatCache : public AbstractDataCache
{
public:
    FloatCache(const DinSocket *socket=0);
    ~FloatCache();
    virtual FloatCache* getDerived();
    void setData(double d);
    double* getData(int* size=0);
    void clear();

protected:
    void floatValue();
    void intValue();
    void getLoopedCache();
    void math(eMathOp op);
    void modulo();
    void container();
    void stepup();

private:
    double *data;
    int arraysize;
};

class IntCache : public AbstractDataCache
{
public:
    IntCache(const DinSocket *socket=0);
    ~IntCache();
    virtual IntCache* getDerived();
    void setData(int d);
    int* getData(int* size=0);
    int getSingleData();
    void clear();

protected:
    void composeArray();
    void intValue();
    void getLoopedCache();
    void add();
    void subtract();
    void multiply();
    void divide();
    void math(eMathOp op);
    void modulo();
    void container();
    void stepup();
    void glShader();

private:
    int singleData;
    int *data;
    int arraysize;
};

class VectorForeachCacheThread : public QThread
{
public:
    VectorForeachCacheThread(const DinSocket *socket, VertexList *array, int work_start, int work);
    ~VectorForeachCacheThread();
    int getStep();

protected:
    void run();

private:
    const DinSocket *socket;
    VertexList *array;
    int work_start, work, step;
};

class VectorCache : public AbstractDataCache
{
public:
    VectorCache(const DinSocket *socket=0);
    ~VectorCache();
    virtual VectorCache* getDerived();
    void setData(Vector d);
    VertexList* getData();
    Vector getSingleData();
    void clear();

protected:
    void composeArray();
    void vectorValue();
    void floattovector();
    void setArray();
    void forloop();
    void getLoopedCache();
    void container();
    void stepup();
    void foreachloop();

private:
    VertexList *data;
    Vector singleVector;
};

class VectorCacheThreaded : public QThread
{
public:
    VectorCacheThreaded(const DinSocket *socket=0);
    VertexList* getData();

protected:
    void run();

private:
    const DinSocket *socket;
    VertexList *data;
};

#endif /* end of include guard: CACHE_DATA_77LKPR3V */
