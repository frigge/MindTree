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

#include "cache_data.h"
#include "cstdlib"
#include "source/data/scene/object.h"

SceneCache::SceneCache(DoutSocket *socket)
    : AbstractDataCache(socket)
{
    cacheInputs();
}

SceneCache* SceneCache::getDerived()    
{
    return this;
}

QList<Object*> SceneCache::getData()    
{
    return objects;
}

void SceneCache::composeArray()    
{
}

void SceneCache::composeObject()    
{
    ObjectNode *node = getStart()->getNode()->getDerived<ObjectNode>();

    Object *object = new Object;

    //First Input: Vertex Array
    DinSocket *vertarraygate = node->getInSockets().at(0);
    DoutSocket *varr = vertarraygate->getCntdWorkSocket();
    Vector *vertices = 0;
    int size=0;
    if(varr) vertices = VectorCache(varr).getData(&size);
    object->appendVertices(vertices, size);

    //Second Input: Polygon Array: Array of Vertex Indices
    DinSocket *polyarraygate = node->getInSockets().at(2);
    DoutSocket *parr = polyarraygate->getCntdWorkSocket();
    Polygon *polys=0;
    if(parr) polys = PolygonCache(parr).getData(&size);
    if(polys)object->appendPolygons(polys, size);
    objects.append(object);
}

PolygonCache::PolygonCache(DoutSocket *socket)
    : AbstractDataCache(socket)
{
    cacheInputs();
}

PolygonCache* PolygonCache::getDerived()    
{
    return this;
}

Polygon* PolygonCache::getData(int *size)    
{
    if(size)*size = arraysize;
    return data;
}

void PolygonCache::composePolygon()    
{
    data = new Polygon;
    DinSocketList insockets = getStart()->getNode()->getInSockets();
    data->vertexcount = insockets.size() - 1;
    data->vertices = new int[insockets.size() - 1];
    int i = 0;
    foreach(DinSocket *socket, insockets){
        DoutSocket *cntd = socket->getCntdWorkSocket();
        if(cntd) data->vertices[i] = *(IntCache(cntd).getData());
        i++;
    }
}

void PolygonCache::composeArray()    
{
    DinSocketList insockets = getStart()->getNode()->getInSockets();
    int startsize = insockets.size() - 1;
    data = new Polygon[startsize];
    arraysize = startsize;
    int si = 0;
    foreach(DinSocket *socket, insockets){
        DoutSocket *cntd = socket->getCntdWorkSocket();
        if(cntd) data[si] = *(PolygonCache(cntd).getData(&data[si].vertexcount));
        si++;
    }
}

FloatCache::FloatCache(DoutSocket *socket)
    : AbstractDataCache(socket)
{
    cacheInputs();
}

FloatCache* FloatCache::getDerived()    
{
    return this;
}

float* FloatCache::getData(int *size)    
{
    if(size)*size = arraysize;
    return data;
}

IntCache::IntCache(DoutSocket *socket)
    : AbstractDataCache(socket)
{
    cacheInputs();
}

int* IntCache::getData(int* size)    
{
    if(size)*size = arraysize;
    return data;
}

IntCache *IntCache::getDerived()    
{
    return this;
}

void IntCache::composeArray()    
{
}

void IntCache::intValue()    
{
    data = new int;
    *data = ((IntProperty*)getStart()->getNode()->getInSockets().first()->getProperty())->getValue();
    arraysize = 1;
}

VectorCache::VectorCache(DoutSocket *socket)
    : AbstractDataCache(socket)
{
    cacheInputs();
}

Vector* VectorCache::getData(int* size)    
{
    if(size)*size = arraysize;
    return data;
}

VectorCache *VectorCache::getDerived()    
{
    return this;
}

void VectorCache::composeArray()    
{
    ComposeArrayNode *node = getStart()->getNode()->getDerived<ComposeArrayNode>();

    //guess the size of the vector array to be the size of the insocket list
    //=>each input to be a single vector
    arraysize = node->getInSockets().size() - 1;
    int pos = 0;
    data = new Vector[arraysize];
    foreach(DinSocket *socket, node->getInSockets()){
        DoutSocket *cntd = socket->getCntdWorkSocket();
        if(cntd){
            int size=0, i;
            Vector *vecs = VectorCache(cntd).getData(&size);
            if(size>1){
                arraysize += size -1; //1 Item was planned
                data = (Vector*)realloc(data, sizeof(Vector) * arraysize);
            }
            for(i=0; i<size; i++){
                data[pos] = vecs[i];
                pos++;
            }
        }
    }
}

void VectorCache::vectorValue()    
{
    const VectorValueNode *node = getStart()->getNode()->getDerivedConst<VectorValueNode>(); 
    data = new Vector(((VectorProperty*)node->getInSockets().first()->getProperty())->getValue());
    arraysize = 1;
}
