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

#define GLM_SWIZZLE
#include "object.h"

#include "QGLShaderProgram"
#include "QGLBuffer"

#include "cmath"
#include "data/frg.h"
#include "data/project.h"
#include "data/nodes/nodetype.h"
#include "data/nodes/node_db.h"
#include "data/nodes/nodetype.h"
#include "glm/gtc/matrix_transform.hpp"
//#include "source/data/base/properties.h"
//#include "graphics/object_node_vis.h"

using namespace MindTree;

float* VertexList::to1DArray()    
{
    if(getSize()==0)
        return nullptr;

    float *arr = new float[getSize() * 3];
    DataChunk<glm::vec3, 10000> *f = firstChunk;
    for(int i=0; i<getSize() * 3; i += 3){
        arr[i] = f->data[(i/3)%10000].x;
        arr[i+1] = f->data[(i/3)%10000].y;
        arr[i+2] = f->data[(i/3)%10000].z;
        if(!(((i/3)+1)%10000)) f = f->next;
    }
    return arr;
}

unsigned int* PolygonList::getPolySizes()    
{
    if(getSize() == 0)
        return nullptr;

    unsigned int *arr = new unsigned int[getSize()];
    DataChunk<Polygon, 100> *f = firstChunk;
    for(int i=0; i<getSize(); i++) {
        arr[i] = f->data[i%100].size();
        if(!(i+1)%100) f = f->next;
    }
    return arr;
}

unsigned int** PolygonList::getPolyIndices()    
{
    unsigned int **indices = new unsigned int*[getSize()];
    DataChunk<Polygon, 100> *f = firstChunk;
    for(int i=0; i<getSize(); i++) {
        indices[i] = new unsigned int[f->data[i%100].size()];
        for(int j=0; j<f->data[i%100].size(); j++)
            indices[i][j] = f->data[i%100].verts()[j];
        if(!(i+1)%100) f = f->next;
    }
    return indices;
}

void PolygonList::append(Polygon d)    
{
    FRGDataList<Polygon, 100>::append(d);
    for(int i=0; i<d.size(); i++){
        if(vertex_map.size() < d.verts()[i]+1) vertex_map.resize(d.verts()[i]+1);
        vertex_map[d.verts()[i]].push_back(getSize()-1);
    }
}

intVecList_t& PolygonList::getVertexMap()    
{
    return vertex_map;
}

CustomShader::CustomShader()
{
}

CustomShader::~CustomShader()
{
}

AbstractTransformable::AbstractTransformable(eObjType t)
    : parent(nullptr), center(0, 0, 0), type(t)
{
}

AbstractTransformable::~AbstractTransformable()
{
    if(parent)parent->removeChild(shared_from_this());
}

AbstractTransformable::eObjType AbstractTransformable::getType()    
{
    return type;
}

void AbstractTransformable::removeChild(std::shared_ptr<AbstractTransformable> child)    
{
    children.remove(child);
}

void AbstractTransformable::setParent(std::shared_ptr<AbstractTransformable> p)    
{
    parent = p;
    parent->addChild(shared_from_this());
}

std::shared_ptr<AbstractTransformable> AbstractTransformable::getParent()    
{
    return parent;
}

void AbstractTransformable::addChild(std::shared_ptr<AbstractTransformable> obj)    
{
    children.push_back(obj);
}

void AbstractTransformable::addChildren(std::list<std::shared_ptr<AbstractTransformable>>objs)    
{
    for(auto obj : objs)
        children.push_back(obj);
}

std::list<std::shared_ptr<AbstractTransformable>> AbstractTransformable::getChildren()    
{
    return children; 
}

std::string AbstractTransformable::getName()
{
    return name;
}

void AbstractTransformable::setName(std::string value)
{
    name = value;
}

glm::mat4 AbstractTransformable::getTransformation()
{
    return transformation;
}

//void AbstractTransformable::setTransformation(glm::mat4 value)
//{
//    transformation = value;
//}

glm::vec3 AbstractTransformable::getCenter()    
{
    return center;
}

void AbstractTransformable::setCenter(glm::vec3 c)    
{
    center = c;
    glm::mat4 trans;
    trans = glm::lookAt(getPosition(), c, glm::vec3(0, 1, 0));
    transformation = glm::inverse(trans);
}

void AbstractTransformable::setCenter(double x, double y, double z)    
{
    setCenter(glm::vec3(x, y, z));
}

glm::vec3 AbstractTransformable::getPosition()    
{
    return glm::vec4(transformation[3]).xyz();
}

void AbstractTransformable::setPosition(glm::vec3 pos)    
{
    glm::vec3 dist = pos - getPosition();
    center += dist;
    transformation = glm::translate(transformation, dist);
}

void AbstractTransformable::setPosition(double x, double y, double z)    
{
    setPosition(glm::vec3(x, y, z));
}

void AbstractTransformable::translate(glm::vec3 dist)    
{
    setPosition(getPosition() + dist);
}

void AbstractTransformable::translate(double x, double y, double z)    
{
    translate(glm::vec3(x, y, z)); 
}

void AbstractTransformable::posAroundCenter(glm::vec3 newPos)    
{
    glm::mat4 mat = glm::lookAt(newPos, center, glm::vec3(0, 1, 0)); 
    transformation = glm::inverse(mat);
}

void AbstractTransformable::moveToCenter(double fac)    
{
   double dist = glm::vec3(center - getPosition()).length();
   transformation = glm::translate(transformation, glm::vec3(0, 0, dist*fac));
}

double AbstractTransformable::getRotX()    
{
    glm::vec4 x = transformation * glm::vec4(0, 1, 0, 0);
    return acos(glm::dot(glm::vec3(0, 1, 0), glm::normalize(glm::vec3(0, x.y, 0))));
}

void AbstractTransformable::applyTransform(glm::mat4 &transform)    
{
    transformation = transform * transformation;
}

glm::mat4 AbstractTransformable::getWorldTransformation()    
{
    glm::mat4 trans = transformation;
    auto p = parent;
    while(p){
        trans = p->getTransformation() * trans;
        p = p->getParent();
    }
    return trans;
}

ObjectData::ObjectData(ObjectData::eDataType t)
    : type(t)
{
}

ObjectData::~ObjectData()
{
}

ObjectData::eDataType ObjectData::getType()    
{
    return type;
}

void ObjectData::setType(ObjectData::eDataType t)
{
    type = t; 
}

MeshData::MeshData()
    : ObjectData(MESH)
{
}

MeshData::~MeshData()
{
}

void MeshData::addProperty(std::string name, MindTree::Property prop)    
{
    properties[name] = prop; 
}

MindTree::PropertyMap& MeshData::getPropertyMap()    
{
    return properties;
}

std::string MeshData::getName()    
{
    return name;
}

void MeshData::computeFaceNormals()    
{
    auto facenormals = std::make_shared<VertexList>();
    auto polygons = getProperty<std::shared_ptr<PolygonList>>("polygon");
    auto vertices = getProperty<std::shared_ptr<VertexList>>("P");
    for(int i=0; i<polygons->getSize(); i++){
        glm::vec3 normal;
        Polygon poly = (*polygons)[i];
        for(int j=2; j<poly.size(); j++){
           glm::vec3 vec1 = (*vertices)[poly.verts()[j-1]] - (*vertices)[poly.verts()[0]];
           glm::vec3 vec2 = (*vertices)[poly.verts()[j]] - (*vertices)[poly.verts()[j-1]];
           glm::vec3 cross = glm::cross(vec1, vec2);
           normal += cross;
        }
        facenormals->append(glm::normalize(normal));
    }
    addProperty("poly_normal", facenormals);
}

void MeshData::computeVertexNormals()    
{
    computeFaceNormals();
    auto vertexnormals = std::make_shared<VertexList>();
    auto polygons = getProperty<std::shared_ptr<PolygonList>>("polygon");
    auto facenormals = getProperty<std::shared_ptr<VertexList>>("poly_normal");
    intVecList_t vertexMap = polygons->getVertexMap();
    for(auto vector : vertexMap){
        glm::vec3 normal;
        for(auto i : vector){
            glm::vec3 vec2 = (*facenormals)[i];
            if(glm::dot(normal, vec2) < 0) vec2 *= -1;
            normal += vec2;
        }
        vertexnormals->append(glm::normalize(normal));
    }
    addProperty("N", vertexnormals);
}

Object::Object()
    : AbstractTransformable(GEO)
{
}

Object::~Object()
{
}

void Object::addProperty(std::string name, MindTree::Property prop)    
{
    properties[name] = prop; 
}

const MindTree::PropertyMap& Object::getPropertyMap()    
{
    return properties; 
}

std::shared_ptr<ObjectData> Object::getData()
{
    return data;
}

void Object::setData(std::shared_ptr<ObjectData> value)
{
    data = value;
}

void Object::makeInstance(std::shared_ptr<Object> obj)
{
}

Group::Group()
{
}

Group::Group(const Group &grp)
{
    addMembers(grp.getMembers());
}

Group::~Group()
{
}

void Group::addMember(std::shared_ptr<AbstractTransformable> trans)    
{
    members.push_back(trans);
}

void Group::addMembers(std::list<std::shared_ptr<AbstractTransformable>> list)    
{
    members.merge(list);
}

std::list<std::shared_ptr<AbstractTransformable>> Group::getMembers()    const
{
    return members;
}

std::list<std::shared_ptr<Object>> Group::getGeometry()
{
    std::list<std::shared_ptr<Object>> objs;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::GEO)
            objs.push_back(std::static_pointer_cast<Object>(obj));
    return objs;
}

std::list<std::shared_ptr<Camera>> Group::getCameras()    
{
    std::list<std::shared_ptr<Camera>> cams;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::CAMERA)
            cams.push_back(std::static_pointer_cast<Camera>(obj));
    return cams;
}

std::list<std::shared_ptr<Light>> Group::getLights()
{
    std::list<std::shared_ptr<Light>> lights;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::LIGHT)
            lights.push_back(std::static_pointer_cast<Light>(obj));
    return lights;
}

Camera::Camera()
    : AbstractTransformable(CAMERA), fov(45), perspective(true),
    near(.1), far(1000)
{
    setPosition(0, 10, -10);
    setCenter(0, 0, 0);
}

Camera::~Camera()
{
}

void Camera::setProjection(double aspect)    
{
    projection = glm::perspective(fov, aspect, near, far); 
}

glm::mat4 Camera::getProjection()
{
    return projection;
}

glm::mat4 Camera::getViewMatrix()    
{
    return glm::inverse(getTransformation());
}

Light::Light()
    : AbstractTransformable(LIGHT)
{
}

Light::~Light()
{
}

Scene::Scene()
{
}

Scene::~Scene()
{
    //FRG::CurrentProject->removeScene(this);
}

std::string Scene::getName()    
{
    return name;
}

void Scene::setName(std::string n)    
{
    name = n;
}

std::list<Object*> Scene::getObjects()    
{
    return objects;
}

void Scene::setObjects(std::list<Object*> objs)    
{
    objects = objs;
}

std::list<Light*> Scene::getLights()    
{
    return lights;
}

void Scene::setLights(std::list<Light*> lights)  
{
    this->lights = lights;
}

std::list<Camera*> Scene::getCameras()    
{
    return cameras;
}

void Scene::setCameras(std::list<Camera*> cams)    
{
    cameras = cams;
}

PolygonNode::PolygonNode(bool raw)
    : DNode("Object")
{
    setType("POLYGONNODE");
    if(!raw){
        new DoutSocket("Polygon", POLYGON, this);
        setDynamicSocketsNode(IN);
    }
}

PolygonNode::PolygonNode(const PolygonNode &node)
    : DNode(node)
{
}

ObjectDataNodeBase::ObjectDataNodeBase(std::string name)
    : DNode(name)
{
}

ObjectDataNodeBase::ObjectDataNodeBase(const ObjectDataNodeBase &node)
    : DNode(node)
{
}

std::shared_ptr<ObjectData> ObjectDataNodeBase::getObjectData()    
{
    return data;
}

ObjectDataNodeBase::~ObjectDataNodeBase()
{
}

void ObjectDataNodeBase::setObjectData(std::shared_ptr<ObjectData> d)    
{
    data = d;
}

ComposeMeshNode::ComposeMeshNode(bool raw)
    : ObjectDataNodeBase("Mesh")
{
    setObjectData(std::make_shared<MeshData>());
    setNodeType("COMPOSEMESHNODE");
    if(!raw){
        vertSocket = new DAInSocket("Vertices", VECTOR, this);
        polySocket = new DAInSocket("Polygons", POLYGON, this);
        new DoutSocket("Data", OBJDATA, this);
    }
}

ComposeMeshNode::ComposeMeshNode(const ComposeMeshNode &node)
    : ObjectDataNodeBase(node)
{
}

DAInSocket* ComposeMeshNode::getVertSocket()    
{
    return vertSocket;
}

DAInSocket* ComposeMeshNode::getPolySocket()    
{
    return polySocket;
}

//std::shared_ptr<MeshData> ComposeMeshNode::getObjectData()    
//{
//    return std::static_pointer_cast<MeshData>(ObjectDataNodeBase::getObjectData());
//}

AbstractTransformableNode::AbstractTransformableNode(std::string name, bool raw)
    : DNode(name)
{
    if(!raw){
        new DoutSocket("Object", SCENEOBJECT, this);
        transSocket = new DinSocket("Translation", VECTOR, this);
        rotSocket = new DinSocket("Rotation", VECTOR, this);
        scaleSocket = new DinSocket("Scale", VECTOR, this);
        new DinSocket("Parent", SCENEOBJECT, this);
    }
}

AbstractTransformableNode::AbstractTransformableNode(const AbstractTransformableNode &node)
    : transformable(nullptr)
{
    //mainData = new SocketNode(node->getMainData());
    //customData = new SocketNode(node->getCustomData());
}

bool AbstractTransformableNode::isTransformable(DNode *node)    
{
    return true;
}

AbstractTransformableNode::~AbstractTransformableNode()
{
}

DinSocket* AbstractTransformableNode::getTransformSocket()    
{
    return transSocket;
}

DinSocket *AbstractTransformableNode::getRotateSocket()    
{
    return rotSocket;
}

DinSocket *AbstractTransformableNode::getScaleSocket()    
{
    return scaleSocket;
}

std::shared_ptr<AbstractTransformable> AbstractTransformableNode::getObject()    
{
    return transformable;
}

void AbstractTransformableNode::setObject(std::shared_ptr<AbstractTransformable> obj)    
{
    transformable = obj;
}
//
//SocketNode* AbstractTransformableNode::getMainData()    const
//{
//    return mainData;
//}
//
//SocketNode* AbstractTransformableNode::getCustomData()    const
//{
//    return customData;
//}
//
//NodeList AbstractTransformableNode::getAllInNodes(NodeList nodes)    
//{
//    nodes = ContainerNode::getAllInNodes(nodes);
//    LLsocket *tmp = getMainData()->getInSocketLlist()->getFirst();
//    do{
//        if(tmp->socket->getArray())
//            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())     
//                nodes = socket->getNode()->getAllInNodes(nodes);
//        else
//            if(tmp->socket->toIn()->getCntdSocket())
//                nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodes(nodes);
//    }while(tmp = tmp->next);
//
//    tmp = getCustomData()->getInSocketLlist()->getFirst();
//    do{
//        if(tmp->socket->getArray())
//            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())     
//                nodes = socket->getNode()->getAllInNodes(nodes);
//        else
//            if(tmp->socket->toIn()->getCntdSocket())
//                nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodes(nodes);
//    }while(tmp = tmp->next);
//    return nodes;
//}

//ConstNodeList AbstractTransformableNode::getAllInNodesConst(ConstNodeList nodes)    const
//{
//    nodes = ContainerNode::getAllInNodesConst(nodes);
//    LLsocket *tmp = getMainData()->getInSocketLlist()->getFirst();
//    do{
//        if(tmp->socket->getArray())
//            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())     
//                nodes = socket->getNode()->getAllInNodesConst(nodes);
//        else
//            if(tmp->socket->toIn()->getCntdSocket())
//                nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodesConst(nodes);
//    }while(tmp = tmp->next);
//
//    tmp = getCustomData()->getInSocketLlist()->getFirst();
//    do{
//        if(tmp->socket->getArray())
//            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())     
//                nodes = socket->getNode()->getAllInNodesConst(nodes);
//        else
//            if(tmp->socket->toIn()->getCntdSocket())
//                nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodesConst(nodes);
//    }while(tmp = tmp->next);
//    return nodes;
//}
//
void AbstractTransformableNode::setNodeName(std::string name)    
{
    DNode::setNodeName(name);
    transformable->setName(name);
}

ObjectNode::ObjectNode(bool raw)
    : AbstractTransformableNode("Object", raw)
{
    setObject(std::make_shared<Object>());
    setNodeType(NodeType("OBJECT"));
    if(!raw){
        setDynamicSocketsNode(IN);
        objDataSocket = new DinSocket("Data", OBJDATA, this);
        fragSocket = new DinSocket("GLSL Fragment Shader", INTEGER, this);
        vertSocket = new DinSocket("GLSL Vertex Shader", INTEGER, this);
        geoSocket = new DinSocket("GLSL Geometry Shader", INTEGER, this);
    }
}

ObjectNode::ObjectNode(const ObjectNode &node)
    : AbstractTransformableNode(node)
{
}

DinSocket* ObjectNode::getFragSocket()    
{
    return fragSocket;
}

DinSocket* ObjectNode::getVertSocket()    
{
    return vertSocket;
}

DinSocket* ObjectNode::getGeoSocket()    
{
    return geoSocket;
}

DinSocket* ObjectNode::getObjDataSocket()    
{
    return objDataSocket;
}

//Object* ObjectNode::getObject()
//{
//    return (Object*)AbstractTransformableNode::getObject();
//}
//
CreateGroupNode::CreateGroupNode(std::string name, bool raw)
    : DNode(name), group(new Group())
{
    setNodeType("GROUP");
    if(!raw){
        new DoutSocket("Group", GROUPDATA, this);
        new DAInSocket("Members", GROUPDATA, this);
        new DAInSocket("Members", SCENEOBJECT, this);
    }
}

CreateGroupNode::CreateGroupNode(const CreateGroupNode &node)
    : DNode(node), group(new Group())
{
}

CreateGroupNode::~CreateGroupNode()
{
}

Group* CreateGroupNode::getGroup()    
{
    return group;
}

CameraNode::CameraNode(bool raw)
    : AbstractTransformableNode("Camera", raw)
{
    setObject(std::make_shared<Camera>());
    setNodeType("CAMERA");
    if(!raw){
        new DinSocket("Perspective", CONDITION, this);
        new DinSocket("FOV", FLOAT, this);
    }
}

CameraNode::CameraNode(const CameraNode &node)
    : AbstractTransformableNode(node)
{
}

//Camera* CameraNode::getObject()    
//{
//    return (Camera*)AbstractTransformableNode::getObject();
//}
//
LightNode::LightNode(std::string name, bool raw)
    : AbstractTransformableNode(name, raw)
{
    setObject(std::make_shared<Light>());
    setNodeType("LIGHT");
    if(!raw){
        new DinSocket("Intensity", FLOAT, this);
        new DinSocket("Color", COLOR, this);
    }
}

LightNode::LightNode(const LightNode &node)
    : AbstractTransformableNode(node)
{
}

LightNode::LightType LightNode::getLightType()    
{
    return lighttype;
}

void LightNode::setLightType(LightNode::LightType t)    
{
    lighttype = t; 
}

//Light* LightNode::getObject()
//{
//    return (Light*)AbstractTransformableNode::getObject();
//}

PointLightNode::PointLightNode(bool raw)
    : LightNode("Point Light", raw)
{
    //setLightType("POINTLIGHT");
}

PointLightNode::PointLightNode(const PointLightNode &node)
    : LightNode(node)
{
}

SpotLightNode::SpotLightNode(bool raw)
    : LightNode("Spot Light", raw)
{
    //setLightType("SPOTLIGHT");
}

SpotLightNode::SpotLightNode(const SpotLightNode &node)
    : LightNode(node)
{
}

DistantLightNode::DistantLightNode(bool raw)
    : LightNode("Distant Light", raw)
{
    setLightType(DISTANTLIGHT);
}

DistantLightNode::DistantLightNode(const DistantLightNode &node)
    : LightNode(node)
{
}
