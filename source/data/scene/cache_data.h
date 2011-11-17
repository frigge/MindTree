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

class Object;
class Polygon;
class SceneCache : public AbstractDataCache
{
public:
    SceneCache(DoutSocket *node);
    QList<Object*> getData();
    virtual SceneCache* getDerived();

protected:
    virtual void composeArray();
    void composeObject();

private:
    QList<Object*> objects;
};

class PolygonCache : public AbstractDataCache
{
public:
    PolygonCache(DoutSocket *socket);
    virtual PolygonCache* getDerived();
    Polygon* getData(int *size=0);

protected:
    void composePolygon();
    void composeArray();

private:
    Polygon *data;
    int arraysize;
};

class FloatCache : public AbstractDataCache
{
public:
    FloatCache(DoutSocket *socket);
    virtual FloatCache* getDerived();
    float* getData(int* size=0);

private:
    float *data;
    int arraysize;
};

class IntCache : public AbstractDataCache
{
public:
    IntCache(DoutSocket *socket);
    virtual IntCache* getDerived();
    int* getData(int* size=0);

protected:
    void composeArray();
    void intValue();

private:
    int *data;
    int arraysize;
};

class VectorCache : public AbstractDataCache
{
public:
    VectorCache(DoutSocket *socket);
    virtual VectorCache* getDerived();
    Vector* getData(int* size=0);

protected:
    void composeArray();
    void vectorValue();

private:
    Vector *data;
    int arraysize;
};


#endif /* end of include guard: CACHE_DATA_77LKPR3V */
