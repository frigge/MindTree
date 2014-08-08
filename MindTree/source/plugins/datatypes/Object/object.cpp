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

#include "cmath"
#include "data/project.h"
#include "data/nodes/nodetype.h"
#include "data/nodes/node_db.h"
#include "data/nodes/nodetype.h"
#include "glm/gtc/matrix_transform.hpp"
//#include "graphics/object_node_vis.h"

using namespace MindTree;
PROPERTY_TYPE_INFO(GroupPtr, "GROUPDATA");
PROPERTY_TYPE_INFO(GeoObjectPtr, "SCENEOBJECT");
PROPERTY_TYPE_INFO(VertexListPtr, "VERTEXLIST");
PROPERTY_TYPE_INFO(PolygonListPtr, "POLYGONLIST");

AbstractTransformable::AbstractTransformable(eObjType t)
    : center(0, 0, 0), type(t)
{
}

AbstractTransformable::AbstractTransformable(const AbstractTransformable &other)
    : MindTree::Object(other),
    type(other.type),
    center(other.center),
    transformation(other.transformation),
    name(other.name)

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
    glm::vec4 pos = transformation * glm::vec4(0, 0, 0, 1);
    return pos.xyz();
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

std::string MeshData::getName()    
{
    return name;
}

void MeshData::computeFaceNormals()    
{
    auto facenormals = std::make_shared<VertexList>();
    auto polygons = getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto vertices = getProperty("P").getData<std::shared_ptr<VertexList>>();

    for(const auto &poly : *polygons) {
        glm::vec3 normal;

        for(int j=2; j<poly.size(); j++){
            uint vec_prev_i = poly.verts()[j - 1];
            uint vec_cur_i = poly.verts()[j];
            uint vec_first_i = poly.verts()[0];

            glm::vec3 vec_prev = vertices->at(vec_prev_i);
            glm::vec3 vec_cur = vertices->at(vec_cur_i);
            glm::vec3 vec_first = vertices->at(vec_first_i);

            glm::vec3 vec1 = vec_cur - vec_prev;
            glm::vec3 vec2 = vec_cur - vec_first;
            glm::vec3 cross = glm::cross(vec2, vec1);

            //do not accidentally kill the normal
            glm::vec3 new_normal = normal + cross;
            if(glm::length(new_normal) > 0)
                normal = new_normal;
        }

        //yet there could still be a corrupt poly and glm crashes in that case
        if(glm::length(normal) > 0)
            normal = glm::normalize(normal);

        facenormals->push_back(normal);
    }
    setProperty("poly_normal", facenormals);
}

void MeshData::computeVertexNormals()    
{
    computeFaceNormals();
    auto vertexnormals = std::make_shared<VertexList>();
    auto verts = getProperty("P").getData<std::shared_ptr<VertexList>>();
    auto polygons = getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto facenormals = getProperty("poly_normal").getData<std::shared_ptr<VertexList>>();
    //loop through all vertices by index
    for(size_t i = 0; i < verts->size(); i++) {
        glm::vec3 normal;
        //loop through all faces by index
        for(size_t j = 0; j < facenormals->size(); j++) {
            //if the polygon contains the currently looked at vertex
            for(int pi : polygons->at(j).verts()) {
                if (pi == i) {
                    //add it to the normal vector
                    glm::vec3 vec2 = (*facenormals)[j];
                    //if(glm::dot(normal, vec2) < 0) vec2 *= -1;
                    glm::vec3 new_normal = normal + vec2;
                    if(glm::length(new_normal) > 0)
                        normal = new_normal;
                    break;
                }
            }
        }
        if(glm::length(normal) > 0)
            vertexnormals->push_back(glm::normalize(normal));
        else
            vertexnormals->push_back(glm::vec3(1, 0, 0));
    }
    setProperty("N", vertexnormals);
}

GeoObject::GeoObject()
    : AbstractTransformable(GEO)
{
}

GeoObject::GeoObject(const GeoObject &other)
    : AbstractTransformable(other), data(other.data)
{
}


GeoObject::~GeoObject()
{
}

MaterialInstancePtr GeoObject::getMaterial()
{
    return _material;
}

void GeoObject::setMaterial(MaterialInstancePtr material)
{
    _material = material;
}

std::shared_ptr<ObjectData> GeoObject::getData()
{
    return data;
}

void GeoObject::setData(std::shared_ptr<ObjectData> value)
{
    data = value;
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

std::list<std::shared_ptr<GeoObject>> Group::getGeometry()
{
    std::list<std::shared_ptr<GeoObject>> objs;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::GEO)
            objs.push_back(std::static_pointer_cast<GeoObject>(obj));
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
    near(.1), far(10000)
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

std::list<GeoObject*> Scene::getObjects()    
{
    return objects;
}

void Scene::setObjects(std::list<GeoObject*> objs)    
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
        new DoutSocket("Polygon", "POLYGON", this);
        setDynamicSocketsNode(DSocket::IN);
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
        new DoutSocket("Data", "OBJDATA", this);
    }
}

ComposeMeshNode::ComposeMeshNode(const ComposeMeshNode &node)
    : ObjectDataNodeBase(node)
{
}

//std::shared_ptr<MeshData> ComposeMeshNode::getObjectData()    
//{
//    return std::static_pointer_cast<MeshData>(ObjectDataNodeBase::getObjectData());
//}

AbstractTransformableNode::AbstractTransformableNode(std::string name, bool raw)
    : DNode(name)
{
    if(!raw){
        new DoutSocket("Object", "SCENEOBJECT", this);
        transSocket = new DinSocket("Translation", "VECTOR", this);
        rotSocket = new DinSocket("Rotation", "VECTOR", this);
        scaleSocket = new DinSocket("Scale", "VECTOR", this);
        new DinSocket("Parent", "SCENEOBJECT", this);
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
    setObject(std::make_shared<GeoObject>());
    setNodeType(NodeType("OBJECT"));
    if(!raw){
        setDynamicSocketsNode(DSocket::IN);
        objDataSocket = new DinSocket("Data", "OBJDATA", this);
        fragSocket = new DinSocket("GLSL Fragment Shader", "INTEGER", this);
        vertSocket = new DinSocket("GLSL Vertex Shader", "INTEGER", this);
        geoSocket = new DinSocket("GLSL Geometry Shader", "INTEGER", this);

        new DoutSocket("Object", "GROUPDATA", this);
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
        new DoutSocket("Group", "GROUPDATA", this);
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
        new DinSocket("Perspective", "CONDITION", this);
        new DinSocket("FOV", "FLOAT", this);
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
        new DinSocket("Intensity", "FLOAT", this);
        new DinSocket("Color", "COLOR", this);
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
