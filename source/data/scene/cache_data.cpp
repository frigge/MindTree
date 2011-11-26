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
#include "ctime"
#include "iostream"
#include "source/data/scene/object.h"

int counter = 0;
void inc_counter(std::string name)
{
    counter++;
    std::cout<<"entering " << name <<" : "<< counter << std::endl;
}

void dec_counter(std::string name)
{
    counter--;
    std::cout<<"leaving " << name << " : "<< counter << std::endl;
}

SceneCache::SceneCache(const DinSocket *socket)
    : AbstractDataCache(socket)
{
    if(getStart())
        cacheInputs();
}

SceneCache::~SceneCache()
{
   clear(); 
}

void SceneCache::clear()    
{
    while(!objects.isEmpty())
        delete objects.takeLast();
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
    DinSocket *varr = node->getInSockets().at(0);
    Vector *vertices = 0;
    int size=0;
    VectorCache *vcache = new VectorCache(varr);
    vertices = vcache->getData(&size);
    object->appendVertices(vertices, size);
    delete vcache;

    //Second Input: Polygon Array: Array of Vertex Indices
    DinSocket *parr = node->getInSockets().at(2);
    Polygon *polys=0;
    PolygonCache *pcache = new PolygonCache(parr);
    polys = pcache->getData(&size);
    if(polys)
    {
        object->appendPolygons(polys, size);
        delete pcache;
    }
    objects.append(object);
}

PolygonCache::PolygonCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0), arraysize(0)
{
    if(getStart())
        cacheInputs();
}

PolygonCache::~PolygonCache()
{
    clear();
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
    data = new Polygon[1];
    DinSocketList insockets = getStart()->getNode()->getInSockets();
    data->vertexcount = insockets.size() - 1;
    data->vertices = new int[insockets.size() - 1];
    int i = 0;
    IntCache *ic;
    foreach(DinSocket *socket, insockets){
        ic = new IntCache(socket);
        data->vertices[i] = *ic->getData();
        delete ic;
        i++;
    }
}

void PolygonCache::clear()    
{
    if(data)
        delete [] data;
    data = 0;
}

void PolygonCache::composeArray()    
{
    DinSocketList insockets = getStart()->getNode()->getInSockets();
    int startsize = insockets.size() - 1;
    data = new Polygon[startsize];
    arraysize = startsize;
    int si = 0;
    PolygonCache *pc=0;
    foreach(DinSocket *socket, insockets){
        pc = new PolygonCache(socket);
        data[si] = *pc->getData(&data[si].vertexcount);
        delete pc;
        si++;
    }
}

FloatCache::FloatCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0), arraysize(0)
{
    if(getStart())
        cacheInputs();
    else{
        data = new double[1];
        arraysize = 1;
        if(socket->getType() != VARIABLE)
            *data = ((FloatProperty*)socket->getProperty())->getValue();
    }
}

FloatCache::~FloatCache()
{
    clear();
}

void FloatCache::clear()
{
    if(data)
        delete [] data;
    data=0;
}

FloatCache* FloatCache::getDerived()    
{
    return this;
}

void FloatCache::setData(double d)    
{
    data = new double[1];
    *data = d;
}

double* FloatCache::getData(int *size)    
{
    if(size)*size = arraysize;
    return data;
}

void FloatCache::floatValue()    
{
    data = new double[1];
    *data = *FloatCache(getStart()->getNode()->getInSockets().first()).getData();
    arraysize = 1;
}

void FloatCache::intValue()    
{
    data = new double[1];
    *data = *IntCache(getStart()->getNode()->getInSockets().first()).getData();
    arraysize = 1;
}

void FloatCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    if(ls->getContainer()->getNodeType() == FOR) {
        if(ls->getContainer()->getSocketOnContainer(getStart()) == ls->getContainer()->getInSockets().at(2)) {
            data = new double[1];
            *data = LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getStep();
            arraysize = 1;
        }
        else{
            FloatCache *cache = new FloatCache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
            double *d = cache->getData(&arraysize);
            data = new double[arraysize];
            for(int i=0; i<arraysize; i++)
                data[i] = d[i];
        }
    }
}

void FloatCache::add()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    FloatCache *fc=0;
    int numcnt = insockets.size() - 1;
    double *values = new double[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() == VARIABLE) {
            fc = new FloatCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new double[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data += values[i];
    arraysize=1;
}

void FloatCache::subtract()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    FloatCache *fc=0;
    int numcnt = insockets.size() - 1;
    double *values = new double[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() == VARIABLE) {
            fc = new FloatCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new double[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data -= values[i];
    arraysize=1;
}

void FloatCache::multiply()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    FloatCache *fc=0;
    int numcnt = insockets.size() - 1;
    double *values = new double[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() == VARIABLE) {
            fc = new FloatCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new double[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data *= values[i];
    arraysize=1;
}

void FloatCache::divide()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    FloatCache *fc=0;
    int numcnt = insockets.size() - 1;
    double *values = new double[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() == VARIABLE) {
            fc = new FloatCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new double[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data /= values[i];
    arraysize=1;
}

IntCache::IntCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0), arraysize(0)
{
    if(getStart())
        cacheInputs();
    else{
        data = new int[1];
        arraysize = 1;
        if(socket->getType() != VARIABLE)
            *data = ((IntProperty*)socket->getProperty())->getValue();
    }
}

IntCache::~IntCache()
{
    clear();
}

void IntCache::clear()
{
    if(data)
        delete [] data;
    data=0;
}

void IntCache::setData(int d)    
{
    data = new int[1];
    *data = d;
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
    data = new int[1];
    *data = *IntCache(getStart()->getNode()->getInSockets().first()).getData();
    arraysize = 1;
}

void IntCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    if(ls->getContainer()->getNodeType() == FOR) {
        if(ls->getContainer()->getSocketOnContainer(getStart()) == ls->getContainer()->getInSockets().at(2)) {
            data = new int[1];
            *data = LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getStep();
            arraysize = 1;
        }
        else{
            IntCache *cache = new IntCache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
            int *d = cache->getData(&arraysize);
            data = new int[arraysize];
            for(int i=0; i<arraysize; i++)
                data[i] = d[i];
        }
    }
}

void IntCache::add()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    IntCache *fc=0;
    int numcnt = insockets.size() - 1;
    int *values = new int[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() != VARIABLE) {
            fc = new IntCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new int[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data += values[i];
    arraysize=1;
}

void IntCache::subtract()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    IntCache *fc=0;
    int numcnt = insockets.size() - 1;
    int *values = new int[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() != VARIABLE) {
            fc = new IntCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new int[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data -= values[i];
    arraysize=1;
}

void IntCache::multiply()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    IntCache *fc=0;
    int numcnt = insockets.size() - 1;
    int *values = new int[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() != VARIABLE) {
            fc = new IntCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new int[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        *data *= values[i];
    arraysize=1;
}

void IntCache::divide()    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    IntCache *fc=0;
    int numcnt = insockets.size() - 1;
    int *values = new int[numcnt];
    int i=0;
    foreach(DinSocket *socket, insockets){
        if (socket->getType() != VARIABLE) {
            fc = new IntCache(socket);
            values[i] = *fc->getData();
            delete fc; 
            i++;
        }
    }
    data = new int[1];
    *data = values[0];
    for(i=1; i<numcnt; i++)
        if(values[i] != 0)
            *data /= values[i];
    arraysize=1;
}

VectorCache::VectorCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0), arraysize(0)
{
    if(getStart())
        cacheInputs();
    else{
        data = new Vector[1];
        arraysize = 1;
        if(socket->getType() != VARIABLE)
            *data = ((VectorProperty*)socket->getProperty())->getValue();
    }
}

VectorCache::~VectorCache()
{
    clear();
}

void VectorCache::clear()
{
    if(data)
        delete [] data;

    data=0;
}

void VectorCache::setData(Vector d)    
{
    data = new Vector[1];
    *data = d;
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

void VectorCache::setArray()    
{
    DNode *node = getStart()->getNode();
    DinSocketList inSockets = node->getInSockets();
    DinSocket *arrSocket = inSockets.at(0);
    DinSocket *valSocket = inSockets.at(1);
    DinSocket *indexSocket = inSockets.at(2);
    
    int index=0;
    Vector *vectors = 0;
    Vector value;
    index = *IntCache(indexSocket).getData();
    VectorCache *arrcache = new VectorCache(arrSocket);
    vectors = arrcache->getData(&arraysize);
    Vector *veccpy = new Vector[arraysize];
    int i;
    if(vectors)
        for(i=0; i<arraysize; i++)
            veccpy[i] = vectors[i];
    delete arrcache;
    vectors = veccpy;
    value = *VectorCache(valSocket).getData();

    if(index >= arraysize)
    {
        arraysize = index + 1;
        vectors = (Vector*)realloc(vectors, sizeof(Vector) * arraysize);
    }
    vectors[index] = value;

    data = vectors;
}

void VectorCache::composeArray()    
{
    ComposeArrayNode *node = getStart()->getNode()->getDerived<ComposeArrayNode>();

    //guess the size of the vector array to be the size of the insocket list
    //=>each input to be a single vector
    arraysize = node->getInSockets().size() - 1;
    int pos = 0;
    data = new Vector[arraysize];
    VectorCache *cache=0;
    foreach(DinSocket *socket, node->getInSockets()){
        int size=0, i;
        cache = new VectorCache(socket);
        data[pos] = *cache->getData(&size);
        delete cache;
        pos++;
    }
}

void VectorCache::vectorValue()    
{
    const VectorValueNode *node = getStart()->getNode()->getDerivedConst<VectorValueNode>(); 
    data = new Vector[1];
    *data = (((VectorProperty*)node->getInSockets().first()->getProperty())->getValue());
    arraysize = 1;
}

void VectorCache::floattovector()    
{
    const DNode *node = getStart()->getNode();
    
    DinSocket *x, *y, *z;
    DinSocketList insockets = node->getInSockets();
   
    x = insockets.at(0); 
    y = insockets.at(1); 
    z = insockets.at(2); 

    double xval, yval, zval;

    xval = *FloatCache(x).getData();
    yval = *FloatCache(y).getData();
    zval = *FloatCache(z).getData();

    arraysize = 1;
    data = new Vector[1];
    *data = Vector(xval, yval, zval);
}

void VectorCache::forloop()    
{
    const ForNode *node = getStart()->getNode()->getDerivedConst<ForNode>(); 
    DinSocket *start, *end, *step;
    DinSocketList insockets = node->getInSockets();

    start = insockets.at(0);
    end = insockets.at(1);
    step = insockets.at(2);

    int startval, endval, stepval;

    startval = *IntCache(start).getData(); 
    endval = *IntCache(end).getData(); 
    stepval = *IntCache(step).getData(); 

    const LoopSocketNode *innode, *outnode;
    innode = node->getInputs()->getDerivedConst<LoopSocketNode>();
    outnode = node->getOutputs()->getDerivedConst<LoopSocketNode>();

    double stepping;
    DinSocket *lin = node->getSocketInContainer(getStart())->toIn();
    LoopCache *c = LoopCacheControl::loop(node);
    VectorCache *vcache = 0;
    for(stepping = startval; stepping < endval; stepping += stepval) {
        c->setStep(stepping);
        vcache = new VectorCache(lin);
        c->addData(vcache);
    }

    Vector *tmpvecs=0;
    VectorCache *finalvc = (VectorCache*)c->getCache();
    if(finalvc) {
        tmpvecs = finalvc->getData(&arraysize);
        data = new Vector[arraysize];
        int i;
        for(i=0; i<arraysize; i++)
            data[i] = tmpvecs[i];
    
    }
    LoopCacheControl::del(node);
}

void VectorCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    VectorCache* loopedCache = (VectorCache*)LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getCache();
    if(loopedCache) {
        Vector *tmpvec = loopedCache->getData(&arraysize);
        data = new Vector[arraysize];
        for(int i=0; i<arraysize; i++)
            data[i] = tmpvec[i];
    }
    else{
        VectorCache *vc = new VectorCache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
        Vector *vectors = vc->getData(&arraysize);
        data = new Vector[arraysize];
        for(int i=0; i<arraysize; i++)
            data[i] = vectors[i];
        delete vectors;
    }
}

