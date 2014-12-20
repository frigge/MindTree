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

#include "cmath"
#include "glm/gtc/matrix_transform.hpp"

#include "lights.h"

#include "object.h"

using namespace MindTree;
PROPERTY_TYPE_INFO(GroupPtr, "GROUPDATA");

PROPERTY_TYPE_INFO(AbstractTransformablePtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(GeoObjectPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(LightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(PointLightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(DistantLightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(SpotLightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(EmptyPtr, "TRANSFORMABLE");

PROPERTY_TYPE_INFO(VertexListPtr, "VERTEXLIST");
PROPERTY_TYPE_INFO(PolygonListPtr, "POLYGONLIST");

AbstractTransformable::AbstractTransformable(eObjType t)
    : center(0, 0, 0), type(t), parent(nullptr)
{
}

AbstractTransformable::AbstractTransformable(const AbstractTransformable &other)
    : MindTree::Object(other),
    type(other.type),
    parent(other.parent),
    center(other.center),
    transformation(other.transformation),
    name(other.name)

{
    
}

AbstractTransformable::~AbstractTransformable()
{
}

AbstractTransformablePtr AbstractTransformable::clone() const
{
    auto *obj = new AbstractTransformable(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
}

AbstractTransformable::eObjType AbstractTransformable::getType()    
{
    return type;
}

void AbstractTransformable::removeChild(AbstractTransformable* child)
{
    for(auto &c : children) {
        if (c.get() == child)
            children.erase(std::find(begin(children), end(children), c));
    }
}

void AbstractTransformable::setParent(AbstractTransformable* p)
{
    parent = p;
    parent->addChild(this);
}

AbstractTransformable* AbstractTransformable::getParent()    
{
    return parent;
}

const AbstractTransformable* AbstractTransformable::getParent() const
{
    return parent;
}

void AbstractTransformable::addChild(AbstractTransformable *obj)    
{
    children.push_back(std::shared_ptr<AbstractTransformable>(obj));
}

void AbstractTransformable::addChildren(std::vector<AbstractTransformable*>objs)    
{
    for(auto obj : objs)
        addChild(obj);
}

std::vector<std::shared_ptr<AbstractTransformable>> AbstractTransformable::getChildren()    
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
    std::lock_guard<std::mutex> lock(_transformationLock);
    return transformation;
}

//void AbstractTransformable::setTransformation(glm::mat4 value)
//{
//    transformation = value;
//}

glm::vec3 AbstractTransformable::getCenter()    
{
    std::lock_guard<std::mutex> lock(_centerLock);
    return center;
}

void AbstractTransformable::setCenter(glm::vec3 c)    
{
    {
        std::lock_guard<std::mutex> lock(_centerLock);
        center = c;
    }
    glm::mat4 trans;
    trans = glm::lookAt(getPosition(), c, glm::vec3(0, 1, 0));
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        transformation = glm::inverse(trans);
    }
}

void AbstractTransformable::setCenter(double x, double y, double z)    
{
    setCenter(glm::vec3(x, y, z));
}

glm::vec3 AbstractTransformable::getPosition()    
{
    std::lock_guard<std::mutex> lock(_transformationLock);
    glm::vec4 pos = transformation * glm::vec4(0, 0, 0, 1);
    return pos.xyz();
}

void AbstractTransformable::setPosition(glm::vec3 pos)    
{
    glm::vec3 dist = pos - getPosition();
    {
        std::lock_guard<std::mutex> lock(_centerLock);
        center += dist;
    }
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        transformation = glm::translate(transformation, dist);
    }
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
    glm::mat4 mat;
    {
        std::lock_guard<std::mutex> lock(_centerLock);
        mat = glm::lookAt(newPos, center, glm::vec3(0, 1, 0)); 
    }
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        transformation = glm::inverse(mat);
    }
}

void AbstractTransformable::moveToCenter(double fac)    
{
    double dist;
    {
        std::lock_guard<std::mutex> lock(_centerLock);
        dist = glm::vec3(center - getPosition()).length();
    }
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        transformation = glm::translate(transformation, glm::vec3(0, 0, dist*fac));
    }
}

double AbstractTransformable::getRotX()    
{
    glm::vec4 x;
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        x = transformation * glm::vec4(0, 1, 0, 0);
    }
    return acos(glm::dot(glm::vec3(0, 1, 0), glm::normalize(glm::vec3(0, x.y, 0))));
}

void AbstractTransformable::applyTransform(glm::mat4 &transform)    
{
    std::lock_guard<std::mutex> lock(_transformationLock);
    transformation = transform * transformation;
}

glm::mat4 AbstractTransformable::getWorldTransformation()    
{
    glm::mat4 trans;
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        trans = transformation;
    }
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

AbstractTransformablePtr GeoObject::clone() const
{
    auto *obj = new GeoObject(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
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

Group::~Group()
{
}

void Group::addMember(std::shared_ptr<AbstractTransformable> trans)    
{
    members.push_back(trans);
}

void Group::addMembers(std::vector<std::shared_ptr<AbstractTransformable>> list)    
{
    members.insert(end(members), begin(list), end(list));
}

std::vector<std::shared_ptr<AbstractTransformable>> Group::getMembers()    const
{
    return members;
}

std::vector<std::shared_ptr<GeoObject>> Group::getGeometry()
{
    std::vector<std::shared_ptr<GeoObject>> objs;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::GEO)
            objs.push_back(std::static_pointer_cast<GeoObject>(obj));
    return objs;
}

std::vector<std::shared_ptr<Camera>> Group::getCameras()    
{
    std::vector<std::shared_ptr<Camera>> cams;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::CAMERA)
            cams.push_back(std::static_pointer_cast<Camera>(obj));
    return cams;
}

std::vector<std::shared_ptr<Light>> Group::getLights()
{
    std::vector<std::shared_ptr<Light>> lights;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::LIGHT)
            lights.push_back(std::static_pointer_cast<Light>(obj));
    return lights;
}

Camera::Camera()
    : AbstractTransformable(CAMERA), fov(45), perspective(true),
    near(.1), far(10000), _width(0), _height(0)
{
    setPosition(0, 10, -10);
    setCenter(0, 0, 0);
}

Camera::Camera(const Camera &other) :
    AbstractTransformable(other),
    projection(other.projection),
    fov{other.fov.load()},
    perspective{other.perspective.load()},
    near{other.near.load()},
    far{other.far.load()}

{
}

Camera::~Camera()
{
}

AbstractTransformablePtr Camera::clone() const
{
    auto *obj = new Camera(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
}

void Camera::setProjection(double aspect)    
{
    std::lock_guard<std::mutex> lock(_projectionLock);
    projection = glm::perspective(fov.load(), aspect, near.load(), far.load()); 
}

glm::mat4 Camera::getProjection()
{
    std::lock_guard<std::mutex> lock(_projectionLock);
    return projection;
}

glm::mat4 Camera::getViewMatrix()    
{
    return glm::inverse(getTransformation());
}

