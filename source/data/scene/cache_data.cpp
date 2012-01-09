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
#include "source/data/code_generator/outputs.h"
#include "source/data/base/frg.h"
#include "source/data/base/project.h"

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
    if(getStart() && !CacheControl::isOutDated(getStart()->getNode())
        && CacheControl::isCached(getStart())){
        objects = ((SceneCache*)CacheControl::getCache(getStart()))->getData();
        return;
    }
    CacheControl::addCache(getStart(), this);
    if(getStart())
        cacheInputs();
}

SceneCache::~SceneCache()
{
    CacheControl::removeCache(this);
    if(isOwner())
       clear(); 
}

Object* SceneCache::newObject(const DNode *node)    
{
    if(FRG::CurrentProject->isCached(node))
        return FRG::CurrentProject->getObject(node);
    else return new Object(node);
}

void SceneCache::clear()    
{
    objects.clear();
}

SceneCache* SceneCache::getDerived()    
{
    return this;
}

QList<Object*> SceneCache::getData()    
{
    setOwner(false);
    return objects;
}

void SceneCache::composeArray()    
{
    foreach(DinSocket *socket, getStart()->getNode()->getInSockets()){
        if(socket == getStart()->getNode()->getInSockets().last())
            continue;
        SceneCache *cache = new SceneCache(socket);
        objects.append(cache->getData());
    }
}

void SceneCache::composeObject()    
{
    ObjectNode *node = getStart()->getNode()->getDerived<ObjectNode>();

    Object *object;
    if(!objects.isEmpty()) object = objects.first();
    else object = newObject(node);

    //TODO: StringCache()

    //1.Vertices
    DinSocket *varr = node->getInSocketLlist()->getLLsocketAt(0)->socket->toIn();
    int size=0;
    VectorCacheThreaded *vcache = new VectorCacheThreaded(varr);

    //2.Poly Index Array
    DinSocket *parr = node->getInSocketLlist()->getLLsocketAt(1)->socket->toIn();
    PolygonCache *pcache = new PolygonCache(parr);

    PolygonList *polys = pcache->getData();
    VertexList *v = vcache->getData();
    object->setPolygons(polys);
    object->setVertices(v);
    delete vcache;
    delete pcache;

    int glfragID, glvertID, glgeoID;
    DinSocket *glsin=0;
    IntCache *glShaderCache=0;
    //3.GLSL Fragment Shader
    glsin = node->getInSocketLlist()->getLLsocketAt(2)->socket->toIn();
    glShaderCache = new IntCache(glsin);
    glfragID = glShaderCache->getSingleData();
    delete glShaderCache;

    //4.GLSL Vertex Shader
    glsin = node->getInSocketLlist()->getLLsocketAt(3)->socket->toIn();
    glShaderCache = new IntCache(glsin);
    glvertID = glShaderCache->getSingleData();
    delete glShaderCache;

    //5.GLSL Geometry Shader
    glsin = node->getInSocketLlist()->getLLsocketAt(4)->socket->toIn();
    glShaderCache = new IntCache(glsin);
    glgeoID = glShaderCache->getSingleData();
    delete glShaderCache;

    object->setGLFragID(glfragID);
    object->setGLVertexID(glvertID);
    object->setGLGeoID(glgeoID);

    objects.append(object);
}

void SceneCache::container()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
    SceneCache *cache = new SceneCache(node->getSocketInContainer(getStart())->toIn());
    QList<Object*> objs = cache->getData();
    Object *object = new Object(node);
    object->addChildren(objs);
    objects.append(object);
}

void SceneCache::stepup()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
    SceneCache *cache = new SceneCache(node->getSocketOnContainer(getStart())->toIn());
    objects = cache->getData();
    delete cache;
}

PolygonCache::PolygonCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0)
{
    setType(AbstractDataCache::POLYGONCACHE);
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

PolygonList* PolygonCache::getData()
{
    setOwner(false);
    return data;
}

Polygon PolygonCache::getSingleData()    
{
   return singlePolygon;
}

void PolygonCache::composePolygon()    
{
    DinSocketList insockets = getStart()->getNode()->getInSockets();
    singlePolygon.vertexcount = insockets.size() - 1;
    singlePolygon.vertices = new int[insockets.size() - 1];
    int i = 0;
    IntCache *ic;
    foreach(DinSocket *socket, insockets){
        if(!socket->getCntdSocket())continue;
        ic = new IntCache(socket);
        singlePolygon.vertices[i] = ic->getSingleData();
        delete ic;
        i++;
    }
}

void PolygonCache::clear()    
{
    if(data && isOwner())
        delete data;
    data = 0;
}

void PolygonCache::composeArray()    
{
    DinSocketList insockets = getStart()->getNode()->getInSockets();
    int startsize = insockets.size() - 1;
    data = new PolygonList;
    PolygonCache *pc=0;
    foreach(DinSocket *socket, insockets){
        if(!socket->getCntdSocket())continue;
        pc = new PolygonCache(socket);
        data->append(pc->getSingleData());
        pc->setOwner(true);
        delete pc;
    }
}

void PolygonCache::container()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
    PolygonCache *cache = new PolygonCache(node->getSocketInContainer(getStart())->toIn());
    data = cache->getData();
    singlePolygon = cache->getSingleData();
    delete cache;
}

void PolygonCache::stepup()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
    PolygonCache *cache = new PolygonCache(node->getSocketOnContainer(getStart())->toIn());
    data = cache->getData();
    singlePolygon = cache->getSingleData();
    delete cache;
}

void PolygonCache::forloop()    
{
    const ForNode *node = getStart()->getNode()->getDerivedConst<ForNode>(); 
    DinSocket *start, *end, *step;
    DinSocketList insockets = node->getInSockets();

    start = insockets.at(0);
    end = insockets.at(1);
    step = insockets.at(2);

    int startval, endval, stepval;

    startval = IntCache(start).getSingleData(); 
    endval = IntCache(end).getSingleData(); 
    stepval = IntCache(step).getSingleData(); 

    const LoopSocketNode *innode, *outnode;
    innode = node->getInputs()->getDerivedConst<LoopSocketNode>();
    outnode = node->getOutputs()->getDerivedConst<LoopSocketNode>();

    double stepping;
    DinSocket *lin = node->getSocketInContainer(getStart())->toIn();
    LoopCache *c = LoopCacheControl::loop(node);
    PolygonCache *pcache = 0;
    for(stepping = startval; stepping < endval; stepping += stepval) {
        c->setStep(stepping);
        pcache = new PolygonCache(lin);
        c->addData(pcache);
    }

    PolygonCache *finalpc = (PolygonCache*)c->getCache();
    if(finalpc) 
        data = finalpc->getData();

    LoopCacheControl::del(node);
}

void PolygonCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    PolygonCache* loopedCache = (PolygonCache*)LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getCache();
    if(loopedCache) {
        data = loopedCache->getData();
        singlePolygon = loopedCache->getSingleData();
    }
    
    else{
        PolygonCache *pc = new PolygonCache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
        data = pc->getData();
        singlePolygon = pc->getSingleData();
        delete pc;
    }
}

void PolygonCache::setArray()    
{
    DNode *node = getStart()->getNode();
    DinSocketList inSockets = node->getInSockets();
    DinSocket *arrSocket = inSockets.at(0);
    DinSocket *valSocket = inSockets.at(1);
    DinSocket *indexSocket = inSockets.at(2);
    
    int index=0;
    Polygon value;
    IntCache *indexCache = new IntCache(indexSocket);
    index = indexCache->getSingleData();
    delete indexCache;
    PolygonCache *arrcache = new PolygonCache(arrSocket);
    data = arrcache->getData();
    delete arrcache;
    PolygonCache *valueCache = new PolygonCache(valSocket);
    value = valueCache->getSingleData();
    delete valueCache;

    if(!data) data = new PolygonList;
    (*data)[index] = value;
}

FloatCache::FloatCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0), arraysize(0)
{
    setType(AbstractDataCache::FLOATCACHE);
    if(getStart())
        cacheInputs();
    else{
        data = new double[1];
        data[0] = 0;
        arraysize = 1;
        if(socket->getType() != VARIABLE) {
            if(socket->getType() == FLOAT)
                data[0] = ((FloatProperty*)socket->getProperty())->getValue();
            else if(socket->getType() == INTEGER)
                data[0] = ((IntProperty*)socket->getProperty())->getValue();
        }
    }
}

FloatCache::~FloatCache()
{
    clear();
}

void FloatCache::clear()
{
    if(data && isOwner())
        delete [] data;
    data=0;
}

FloatCache* FloatCache::getDerived()    
{
    return this;
}

void FloatCache::setData(double d)    
{
    if(data) delete[] data;
    data = new double[1];
    *data = d;
}

double* FloatCache::getData(int *size)    
{
    if(size)*size = arraysize;
    setOwner(false);
    return data;
}

void FloatCache::floatValue()    
{
    DinSocket *tmp = getStart()->getNode()->getInSockets().first();
    FloatCache *cache = new FloatCache(tmp);
    data = cache->getData();
    delete cache;
    arraysize = 1;
}

void FloatCache::intValue()    
{
    data = new double[1];
    IntCache *ic = new IntCache(getStart()->getNode()->getInSockets().first());
    *data = ic->getSingleData();
    delete ic;
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
            data = cache->getData(&arraysize);
            delete cache;
        }
    }
}

void FloatCache::math(eMathOp op)    
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    FloatCache *fc=0;
    int i=0;
    foreach(DinSocket *socket, insockets){
        if(i == 0) {
            fc = new FloatCache(socket);
            data = fc->getData();
            delete fc; 
        }
        else {
            if(!socket->getCntdSocket() && socket->getType() == VARIABLE)
                continue;
            fc = new FloatCache(socket);
            switch(op)
            {
                case OPADD:
                    data[0] += fc->getData()[0];
                    break;
                case OPSUBTRACT:
                    data[0] -= fc->getData()[0];
                    break;
                case OPMULTIPLY:
                    data[0] *= fc->getData()[0];
                    break;
                case OPDIVIDE:
                    data[0] /= fc->getData()[0];
                    break;
            }
            fc->setOwner(true);
            delete fc; 
        }
        i++;
    }
    arraysize=1;
}

void FloatCache::modulo()
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    IntCache *valuecache=new IntCache(insockets.at(0));
    IntCache *divcache=new IntCache(insockets.at(1));

    data = new double[1];
    *data = valuecache->getSingleData() % divcache->getSingleData();
    arraysize = 1;
}

void FloatCache::container()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
    FloatCache *cache = new FloatCache(node->getSocketInContainer(getStart())->toIn());
    data = cache->getData(&arraysize);
    delete cache;
}

void FloatCache::stepup()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
    if(node->getNodeType() == FOREACHNODE
        &&getStart() == getStart()->getNode()->getOutSocketLlist()->getLLsocketAt(0)->socket) {
        data = new double[1];
        *data = ((VectorForeachCacheThread*)QThread::currentThread())->getStep();
        return;
    }
    FloatCache *cache = new FloatCache(node->getSocketOnContainer(getStart())->toIn());
    data = cache->getData(&arraysize);
    delete cache;
}

IntCache::IntCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0), arraysize(0), singleData(0)
{
    setType(AbstractDataCache::INTEGERCACHE);
    if(getStart())
        cacheInputs();
    else{
        arraysize = 1;
        if(socket->getType() != VARIABLE)
            singleData = ((IntProperty*)socket->getProperty())->getValue();
    }
}

IntCache::~IntCache()
{
    clear();
}

void IntCache::clear()
{
    if(data && isOwner())
        delete [] data;
    data=0;
}

void IntCache::setData(int d)    
{
    if(data) delete[] data;
    data = 0;
    singleData=d;
}
int* IntCache::getData(int* size)    
{
    if(size)*size = arraysize;
    setOwner(false);
    return data;
}

int IntCache::getSingleData()    
{
    return singleData;
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
    singleData = IntCache(getStart()->getNode()->getInSockets().first()).getSingleData();
    arraysize = 1;
}

void IntCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    if(ls->getContainer()->getNodeType() == FOR) {
        if(ls->getContainer()->getSocketOnContainer(getStart()) == ls->getContainer()->getInSocketLlist()->getLLsocketAt(2)->socket) {
            singleData = LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getStep();
            arraysize = 1;
        }
        else{
            IntCache *cache = new IntCache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
            data = cache->getData(&arraysize);
            singleData = cache->getSingleData();
            delete cache;
        }
    }
}

void IntCache::math(eMathOp op)    
{
    DNode *node = getStart()->getNode();
    DSocketList *insockets = node->getInSocketLlist();
    IntCache *fc=0;
    LLsocket *first = insockets->getFirst();
    LLsocket *soc = first;
    while(soc) {
        DinSocket *socket = soc->socket->toIn();
        if(soc == first){
            fc = new IntCache(socket);
            singleData = fc->getSingleData();
            delete fc; 
        }
        else {
            if(!socket->getCntdSocket() && socket->getType() == VARIABLE) {
                soc = soc->next;
                continue;
            }
            fc = new IntCache(socket);
            switch(op) {
                case OPADD:
                    singleData += fc->getSingleData();
                    break;
                case OPSUBTRACT:
                    singleData -= fc->getSingleData();
                    break;
                case OPMULTIPLY:
                    singleData *= fc->getSingleData();
                    break;
                case OPDIVIDE:
                    singleData /= fc->getSingleData();
                    break;
            }
            fc->setOwner(true);
            delete fc; 
        }
        soc = soc->next;
    }
    arraysize=1;
}

void IntCache::modulo()
{
    DNode *node = getStart()->getNode();
    DinSocketList insockets = node->getInSockets();
    IntCache *valuecache=new IntCache(insockets.at(0));
    IntCache *divcache=new IntCache(insockets.at(1));

    singleData = valuecache->getSingleData() % divcache->getSingleData();
    arraysize = 1;
}

void IntCache::container()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
    IntCache *cache = new IntCache(node->getSocketInContainer(getStart())->toIn());
    data = cache->getData(&arraysize);
    singleData = cache->getSingleData();
    delete cache;
}

void IntCache::stepup()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
    if(node->getNodeType() == FOREACHNODE
        &&getStart() == getStart()->getNode()->getOutSocketLlist()->getLLsocketAt(0)->socket) {
        singleData = ((VectorForeachCacheThread*)QThread::currentThread())->getStep();
        return;
    }
    IntCache *cache = new IntCache(node->getSocketOnContainer(getStart())->toIn());
    data = cache->getData(&arraysize);
    singleData = cache->getSingleData();
    delete cache;
}

void IntCache::glShader()    
{
    const GLSLOutputNode *node = getStart()->getNode()->getDerivedConst<GLSLOutputNode>();
    singleData = node->compile();
    arraysize=1;
}

VectorCache::VectorCache(const DinSocket *socket)
    : AbstractDataCache(socket), data(0)
{
    setType(AbstractDataCache::VECTORCACHE);
    if(getStart())
        cacheInputs();
    else{
        if(socket->getType() != VARIABLE)
            singleVector = ((VectorProperty*)socket->getProperty())->getValue();
    }
}

VectorCache::~VectorCache()
{
    clear();
}

void VectorCache::clear()
{
    if(data && isOwner())
        delete data;

    data=0;
}

void VectorCache::setData(Vector d)    
{
    singleVector = d;
}

VertexList* VectorCache::getData()
{
    setOwner(false);
    return data;
}

Vector VectorCache::getSingleData()    
{
    return singleVector;
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
    Vector value;
    IntCache *indexCache = new IntCache(indexSocket);
    index = indexCache->getSingleData();
    delete indexCache;
    VectorCache *arrcache = new VectorCache(arrSocket);
    data = arrcache->getData();
    delete arrcache;
    VectorCache *valueCache = new VectorCache(valSocket);
    value = valueCache->getSingleData();
    delete valueCache;

    if(!data)data = new VertexList();
    (*data)[index] = value;
}

void VectorCache::composeArray()    
{
    ComposeArrayNode *node = getStart()->getNode()->getDerived<ComposeArrayNode>();

    int pos = 0;
    data = new VertexList;
    VectorCache *cache=0;
    foreach(DinSocket *socket, node->getInSockets()){
        if(pos == node->getInSockets().size() - 2)return;
        cache = new VectorCache(socket);
        (*data)[pos] = cache->getSingleData();
        delete cache;
        pos++;
    }
}

void VectorCache::vectorValue()    
{
    const VectorValueNode *node = getStart()->getNode()->getDerivedConst<VectorValueNode>(); 
    VectorCache *vcache = new VectorCache(node->getInSockets().first());
    singleVector = vcache->getSingleData();
    delete vcache;
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

    FloatCache *xcache, *ycache, *zcache;
    xcache = new FloatCache(x);
    ycache = new FloatCache(y);
    zcache = new FloatCache(z);
    xval = xcache->getData()[0];
    yval = ycache->getData()[0];
    zval = zcache->getData()[0];

    xcache->setOwner(true);
    ycache->setOwner(true);
    zcache->setOwner(true);
    delete xcache;
    delete ycache;
    delete zcache;

    singleVector = Vector(xval, yval, zval);
    
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

    startval = IntCache(start).getSingleData(); 
    endval = IntCache(end).getSingleData(); 
    stepval = IntCache(step).getSingleData(); 

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

    VectorCache *finalvc = (VectorCache*)c->getCache();
    if(finalvc) 
        data = finalvc->getData();

    LoopCacheControl::del(node);
}

VectorForeachCacheThread::VectorForeachCacheThread(const DinSocket *socket, VertexList *array, int work_start, int work)
    : socket(socket), array(array), work_start(work_start), work(work), step(0)
{
    start();
}

VectorForeachCacheThread::~VectorForeachCacheThread()
{
    wait();
}

void VectorForeachCacheThread::run()    
{
    for(step=work_start; step<work_start + work; step++){
        (*array)[step] = VectorCache(socket).getSingleData();
    }
}

int VectorForeachCacheThread::getStep()    
{
    return step; 
}

void VectorCache::foreachloop()    
{
    const ForeachNode *node = getStart()->getNode()->getDerivedConst<ForeachNode>(); 
    DinSocket *arrayin;
    DinSocketList insockets = node->getInSockets();
    arrayin = insockets.at(0);

    VectorCache *vcache = new VectorCache(arrayin);
    data = vcache->getData();
    delete vcache;

    int array_size = data->getSize();
    int thread_count = QThread::idealThreadCount();
    int thread_work = array_size/thread_count;
    const DinSocket *newValSocket = node->getOutputs()->getInSockets().first();
    VectorForeachCacheThread *threadptr[thread_count];

    int curr_thread_work = 0;
    for(int thread=0; thread<thread_count; thread++) {
        if(thread == thread_count -1) 
            curr_thread_work = thread_work + (array_size%thread_count);
        else
            curr_thread_work = thread_work;
        threadptr[thread] = new VectorForeachCacheThread(newValSocket, data, thread*thread_work, curr_thread_work);
    }
    for(int thread=0; thread<thread_count; thread++)
        delete threadptr[thread];
}

void VectorCache::getLoopedCache()    
{
    LoopSocketNode *ls = getStart()->getNode()->getDerived<LoopSocketNode>();
    VectorCache* loopedCache = (VectorCache*)LoopCacheControl::loop(ls->getContainer()->getDerivedConst<LoopNode>())->getCache();
    if(loopedCache) 
        data = loopedCache->getData();
    
    else{
        VectorCache *vc = new VectorCache(ls->getContainer()->getSocketOnContainer(getStart())->toIn());
        data = vc->getData();
        singleVector = vc->getSingleData();
        delete vc;
    }
}

void VectorCache::container()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
    VectorCache *cache = new VectorCache(node->getSocketInContainer(getStart())->toIn());
    data = cache->getData();
    singleVector = cache->getSingleData();
    delete cache;
}

void VectorCache::stepup()    
{
    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
    VectorCache *cache = new VectorCache(node->getSocketOnContainer(getStart())->toIn());
    data = cache->getData();
    singleVector = cache->getSingleData();
    delete cache;
}

VectorCacheThreaded::VectorCacheThreaded(const DinSocket *socket)
    : socket(socket), data(0)
{
    start();
}

void VectorCacheThreaded::run()    
{
   VectorCache *cache = new VectorCache(socket);
   data = cache->getData(); 
}

VertexList* VectorCacheThreaded::getData()    
{
    wait();
    return data;
}
