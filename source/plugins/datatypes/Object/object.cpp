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

#include "cmath"

#define GLM_FORCE_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"

#include "lights.h"

#include "object.h"

using namespace MindTree;
PROPERTY_TYPE_INFO(GroupPtr, "GROUPDATA");

PROPERTY_TYPE_INFO(AbstractTransformablePtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(ObjectDataPtr, "OBJECTDATA");
PROPERTY_TYPE_INFO(MeshDataPtr, "OBJECTDATA");
PROPERTY_TYPE_INFO(GeoObjectPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(LightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(PointLightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(DistantLightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(SpotLightPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(EmptyPtr, "TRANSFORMABLE");
PROPERTY_TYPE_INFO(CameraPtr, "TRANSFORMABLE");

PROPERTY_TYPE_INFO(Polygon, "POLYGON");

AbstractTransformable::AbstractTransformable(eObjType t)
    : center(0, 0, 0), type(t), _parent(nullptr)
{
}

AbstractTransformable::AbstractTransformable(const AbstractTransformable &other)
    : MindTree::Object(other),
    type(other.type),
    _parent(other._parent),
    center(other.center),
    transformation(other.transformation),
    _name(other._name)
{
    //deep copy children
    for (const auto &child : other._children)
        _children.push_back(child->clone());
}

AbstractTransformable::~AbstractTransformable()
{
}

int AbstractTransformable::getVertexCount() const
{
    int cnt = 0;
    for(auto ch : getChildren())
        cnt += std::static_pointer_cast<GeoObject>(ch)->getVertexCount();
    return cnt;
}

int AbstractTransformable::getPolygonCount() const
{
    int cnt = 0;
    for(auto ch : getChildren())
        cnt += std::static_pointer_cast<GeoObject>(ch)->getPolygonCount();
    return cnt;
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
    for(auto &c : _children) {
        if (c.get() == child)
            _children.erase(std::find(begin(_children), end(_children), c));
    }
}

AbstractTransformable* AbstractTransformable::getParent()    
{
    return _parent;
}

const AbstractTransformable* AbstractTransformable::getParent() const
{
    return _parent;
}

void AbstractTransformable::addChild(AbstractTransformablePtr obj)    
{
    obj->_parent = this;
    _children.push_back(obj);
}

void AbstractTransformable::addChildren(std::vector<AbstractTransformablePtr> objs)    
{
    for(auto obj : objs)
        addChild(obj);
}

std::vector<std::shared_ptr<AbstractTransformable>> AbstractTransformable::getChildren()    
{
    return _children; 
}

std::vector<std::shared_ptr<AbstractTransformable>> AbstractTransformable::getChildren() const 
{
    return _children; 
}

std::string AbstractTransformable::getName()
{
    return _name;
}

void AbstractTransformable::setName(std::string value)
{
    _name = value;
}

glm::mat4 AbstractTransformable::getTransformation()
{
    std::lock_guard<std::mutex> lock(_transformationLock);
    return transformation;
}

void AbstractTransformable::setTransformation(glm::mat4 value)
{
    std::lock_guard<std::mutex> lock(_transformationLock);
    transformation = value;
}

glm::vec3 AbstractTransformable::getCenter()    
{
    std::lock_guard<std::mutex> lock(_centerLock);
    return center;
}

void AbstractTransformable::setCenter(glm::vec3 c)    
{
    std::lock_guard<std::mutex> lock(_centerLock);
    center = c;
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
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        transformation[3] = glm::vec4(pos, 1);
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
    glm::vec3 dist;
    {
        std::lock_guard<std::mutex> lock(_centerLock);
        dist = center - getPosition();
    }

    dist *= fac;
    glm::mat4 translation = glm::translate(glm::mat4(), dist);
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        transformation = translation * transformation;
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

glm::mat4 AbstractTransformable::getWorldTransformation() const
{
    glm::mat4 trans;
    {
        std::lock_guard<std::mutex> lock(_transformationLock);
        trans = transformation;
    }
    if(!_parent)
        return trans;
    else
        return _parent->getWorldTransformation() * trans;
}

void AbstractTransformable::setProperty(std::string name, Property prop)
{
    MindTree::Object::setProperty(name, prop);

    for(auto child : _children) {
        child->setProperty(name, prop);
    }
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

void MeshData::computeVertexNormals()    
{
    auto facenormals = std::make_shared<VertexList>();
    auto polygons = getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto vertices = getProperty("P").getData<std::shared_ptr<VertexList>>();
    auto vertexnormals = std::make_shared<VertexList>();
    vertexnormals->resize(vertices->size());

    for(const auto &poly : *polygons) {
        glm::vec3 normal;

        for(int j=2; j<poly.size(); j++){
            uint vec_prev_i = poly[j - 1];
            uint vec_cur_i = poly[j];
            uint vec_first_i = poly[0];

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

        for (int i : poly) {
            vertexnormals->at(i) += glm::normalize(normal);
        }
    }

    for(auto &n : *vertexnormals)
        n = glm::normalize(n);

    setProperty("N", vertexnormals);
}

int MeshData::getVertexCount() const
{
    if(!hasProperty("P")) return 0;
    auto list = getProperty("P").getData<VertexListPtr>();
    if(!list) return 0;
    return list->size();
}

int MeshData::getPolygonCount() const
{
    if(!hasProperty("polygon")) return 0;
    auto list = getProperty("polygon").getData<PolygonListPtr>();
    if(!list) return 0;

    int cnt = 0;
    for(auto p : *list) {
        cnt += p.size() - 2;
    }
    return cnt;
}

GeoObject::GeoObject()
    : AbstractTransformable(GEO)
{
}

GeoObject::GeoObject(const GeoObject &other)
    : AbstractTransformable(other), data(other.data), _material(other._material)
{
}


GeoObject::~GeoObject()
{
}

int GeoObject::getVertexCount() const
{
    int cnt = AbstractTransformable::getPolygonCount();
    cnt += std::static_pointer_cast<MeshData>(data)->getVertexCount();
    return cnt;
}

int GeoObject::getPolygonCount() const
{
    int cnt = AbstractTransformable::getPolygonCount();
    cnt += std::static_pointer_cast<MeshData>(data)->getPolygonCount();
    return cnt;
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

std::vector<std::shared_ptr<AbstractTransformable>> Group::getMembers() const
{
    return members;
}

std::vector<std::shared_ptr<GeoObject>> Group::getGeometry() const
{
    std::vector<std::shared_ptr<GeoObject>> objs;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::GEO)
            objs.push_back(std::static_pointer_cast<GeoObject>(obj));
    return objs;
}

int Group::getVertexCount() const
{
    int cnt = 0;
    for (auto obj : getMembers()) {
        cnt += obj->getVertexCount();
    }
    return cnt;
}

int Group::getPolygonCount() const
{
    int cnt = 0;
    for (auto obj : getMembers()) {
        cnt += obj->getPolygonCount();
    }
    return cnt;
}

std::vector<std::shared_ptr<Camera>> Group::getCameras() const
{
    std::vector<std::shared_ptr<Camera>> cams;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::CAMERA)
            cams.push_back(std::static_pointer_cast<Camera>(obj));
    return cams;
}

std::vector<std::shared_ptr<Light>> Group::getLights() const
{
    std::vector<std::shared_ptr<Light>> lights;
    for(auto &obj : members)
        if(obj->getType() == AbstractTransformable::LIGHT)
            lights.push_back(std::static_pointer_cast<Light>(obj));
    return lights;
}

void Group::setProperty(std::string name, Property prop)
{
    MindTree::Object::setProperty(name, prop);
    for(auto member : members) {
        member->setProperty(name, prop);
    }
}

Camera::Camera()
    : AbstractTransformable(CAMERA), _fov(glm::radians(45.f)),
    _near(.1), _far(10000), _width(0), _height(0), _aspect(1)
{
    setTransformation(glm::inverse(glm::lookAt(glm::vec3(0, 10, -10), glm::vec3(0), glm::vec3(0, 1, 0))));
}

Camera::Camera(const Camera &other) :
    AbstractTransformable(other),
    _fov{other._fov.load()},
    _aspect{other._aspect.load()},
    _near{other._near.load()},
    _far{other._far.load()},
    _width{other._width.load()},
    _height{other._height.load()}

{
}

Camera::~Camera()
{
}

void Camera::setResolution(int width, int height)
{
    _width = width;
    _height = height;
}

int Camera::getWidth() const
{
    return _width;
}

int Camera::getHeight() const
{
    return _height;
}

AbstractTransformablePtr Camera::clone() const
{
    auto *obj = new Camera(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
}

void Camera::setFov(double fov)
{
    _fov = fov;
}

float Camera::fov() const
{
	return _fov;
}

void Camera::setAspect(double aspect)
{
    _aspect = aspect;
}

void Camera::setNear(double near)
{
    _near = near;
}

void Camera::setFar(double far)
{
    _far = far;
}

glm::mat4 Camera::getProjection()
{
    return glm::perspective(_fov.load(), _aspect.load(), _near.load(), _far.load());
}

glm::mat4 Camera::getViewMatrix()    
{
    return glm::inverse(getWorldTransformation());
}

