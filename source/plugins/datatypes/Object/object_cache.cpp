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

#include "object_cache.h"

#include "data/frg.h"
#include "data/project.h"
#include "../Vector/vector_cache.h"
#include "../Polygon/polygon_cache.h"
//#include "../Int/int_cache.h"
//#include "../Float/float_cache.h"
//#include "source/plugins/io/obj.h"

using namespace MindTree;

ObjectDataCache::ObjectDataCache(const DoutSocket *socket)
    : DataCache(socket)
{
    //if(getStart() && !CacheControl::isOutDated(getStart()->getNode())){
    //    data = ((ObjectDataNodeBase*)getStart()->getNode())->getObjectData();
    //    return;
    //}
    setType(DataCache::OBJECTDATACACHE);
    cacheInputs();
}

ObjectDataCache::~ObjectDataCache()
{
}

//void ObjectDataCache::createMesh()    
//{
//    ////1.Vertices
//    //ComposeMeshNode *node = getStart()->getNode()->getDerived<ComposeMeshNode>();
//    //DAInSocket *vertsIn = node->getVertSocket();
//    //VectorCacheThreaded *vcache = new VectorCacheThreaded(vertsIn);
//
//    ////2.Poly Index Array
//    //DAInSocket *polysIn = node->getPolySocket();
//    //PolygonCache *pcache = new PolygonCache(polysIn);
//
//    //PolygonList *polys = pcache->getData();
//    //VertexList *v = vcache->getData();
//
//    //MeshData *mesh = node->getObjectData();
//    //data = mesh;
//    //mesh->setPolygons(polys);
//    //mesh->setVertices(v);
//    //delete vcache;
//    //delete pcache;
//
//}

//void ObjectDataCache::loadObj()    
//{
//    ObjImportNode *node = getStart()->getNode()->getDerived<ObjImportNode>();
//    ObjImporter objio(node);
//    data = node->getMeshData();
//}
//
//ObjectCache::ObjectCache(const DinSocket *socket)
//    : DataCache(socket)
//{
//    setType(DataCache::OBJECTCACHE);
//    cacheInputs();
//}

//ObjectCache::ObjectCache(const DAInSocket *asocket)
//    : DataCache(asocket)
//{
//    setType(DataCache::OBJECTCACHE);
//    cacheInputs();
//}

ObjectCache::ObjectCache(const DoutSocket *socket)
    : DataCache(socket)
{
    setType(DataCache::OBJECTCACHE);
    cacheInputs();
}

ObjectCache::~ObjectCache()
{
    CacheControl::removeCache(this);
}

//void ObjectCache::clear()    
//{
//
//}
//
//ObjectCache* ObjectCache::getDerived()    
//{
//    return this;
//}
//
//void ObjectCache::composeArray()    
//{
//    //foreach(DinSocket *socket, getStart()->getNode()->getInSockets()){
//    //    if(socket == getStart()->getNode()->getInSockets().last())
//    //        continue;
//    //    ObjectCache *cache = new ObjectCache(socket);
//    //    std::list<Object*> objs = cache->getData();
//    //    objects.merge(objs);
//    //}
//}
//
//void ObjectCache::composeObject()    
//{
//    ObjectNode *node = getStart()->getNode()->getDerived<ObjectNode>();
//
//    Object *object = node->getObject();
//    grp.addMember(object);
//
//    DinSocket *meshIn = node->getObjDataSocket();
//    object->setData(cacheForeign<ObjectData*>(meshIn));
//}
//
//void ObjectCache::createCamera()    
//{
//    Camera* cam = ((CameraNode*)getStart()->getNode())->getObject();
//    grp.addMember(cam);
//}
//
//void ObjectCache::createLight()    
//{
//}
//
//void ObjectCache::composeGroup()    
//{
//    //CreateGroupNode *node = getStart()->getNode()->getDerived<CreateGroupNode>();
//    //Group groups = ObjectCache((DAInSocket*)node->getInSocketLlist()->getLLsocketAt(0)->socket).getData();
//    //Group objects = ObjectCache((DAInSocket*)node->getInSocketLlist()->getLLsocketAt(1)->socket).getData();
//    //grp.addMembers(groups.getMembers());
//    //grp.addMembers(objects.getMembers());
//}
//
//void ObjectCache::container()    
//{
//    const ContainerNode *node = getStart()->getNode()->getDerivedConst<ContainerNode>();
//    cache(node->getSocketInContainer(getStart())->toIn());
//}
//
//void ObjectCache::stepup()    
//{
//    const ContainerNode *node = getStart()->getNode()->getDerivedConst<SocketNode>()->getContainer();
//    cache(node->getSocketOnContainer(getStart())->toIn());
//}

//TODO: move to PolyMeshCache (yet to be created)
//void ObjectCache::transform()    
//{
//    const DNode *node = getStart()->getNode()->getDerivedConst<TransformNode>();
//    DinSocketList insockets = node->getInSockets(); 
//
//    DinSocket *objin, *tin, *rin, *sin;
//    objin = insockets.at(0);
//    tin = insockets.at(1);
//    rin = insockets.at(2);
//    sin = insockets.at(3);
//
//    Vector t, r, s;
//    VectorCache *tcache, *rcache, *scache;
//    tcache = new VectorCache(tin);
//    rcache = new VectorCache(rin);
//    scache = new VectorCache(sin);
//
//    t = tcache->getSingleData();
//    r = rcache->getSingleData();
//    s = scache->getSingleData();
//
//    delete tcache;
//    delete rcache;
//    delete scache;
//
//    ObjectCache *scecache = new ObjectCache(objin);
//    std::list<Object*> objs = scecache->getData();
//    QMatrix4x4 trans;
//    trans.translate(t.x, t.y, t.z);
//    trans.rotate(r.x, QVector3D(1, 0, 0));
//    trans.rotate(r.y, QVector3D(0, 1, 0));
//    trans.rotate(r.z, QVector3D(0, 0, 1));
//    trans.scale(s.x, s.y, s.z);
//    foreach(Object *obj, objs){
//        Object *object = newObject(node);
//        object->setTransformation(trans);
//        object->makeInstance(obj);
//        objects.append(object);
//    }
//}
//
