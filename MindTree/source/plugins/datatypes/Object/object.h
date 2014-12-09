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

#ifndef OBJECT

#define OBJECT

#include "data/nodes/data_node.h"
#include "data/cache_main.h"
#include "data/mtobject.h"
#include "glm/glm.hpp"
#include "QMatrix4x4"
#include "data/python/pyexposable.h"
#include "data/properties.h"
#include "material.h"

class Polygon
{
public:
    Polygon() {}
    Polygon(std::initializer_list<uint> list) : vertices(list) {};
    Polygon(const Polygon &poly){*this = poly;}
    virtual ~Polygon() {}
    Polygon &operator=(const Polygon& poly)
    {
        vertices = poly.vertices;
        return *this;
    }

    void set(std::vector<uint> verts) { vertices = verts; }

    const std::vector<uint>& verts() const { return vertices; }

    int size() const { return vertices.size(); }

private:
    std::vector<uint> vertices;
};

typedef std::vector<glm::vec3> VertexList;
typedef std::shared_ptr<VertexList> VertexListPtr;
typedef std::vector<Polygon> PolygonList;
typedef std::shared_ptr<PolygonList> PolygonListPtr;

class MeshData;
class AbstractTransformable;
typedef std::shared_ptr<AbstractTransformable> AbstractTransformablePtr;
class AbstractTransformable : public MindTree::Object
{
public:
    enum eObjType {
        GEO, CAMERA, LIGHT, EMPTY
    };
    AbstractTransformable(eObjType t);
    virtual ~AbstractTransformable();

    virtual AbstractTransformablePtr clone() const;

    AbstractTransformable::eObjType getType();
    std::string getName();
    void setName(std::string value);

    glm::mat4 getTransformation();
    glm::mat4 getWorldTransformation();
    glm::vec3 getPosition();
    void setPosition(glm::vec3 pos);
    void setPosition(double x, double y, double z);
    glm::vec3 getCenter();
    void setCenter(glm::vec3 c);
    void setCenter(double x, double y, double z);
    double getRotX();
    void applyTransform(glm::mat4 &transform);
    void translate(glm::vec3 dist);
    void translate(double x, double y, double z);
    void posAroundCenter(glm::vec3 newPos);
    void moveToCenter(double fac);

    void setParent(AbstractTransformable* p);
    AbstractTransformable* getParent();
    const AbstractTransformable* getParent() const;
    void addChild(AbstractTransformable* child);
    void addChildren(std::vector<AbstractTransformable*>objs);
    void removeChild(AbstractTransformable *child);
    std::vector<std::shared_ptr<AbstractTransformable>> getChildren();

protected:
    AbstractTransformable(const AbstractTransformable &other);

private:
    eObjType type;
    glm::vec3 center;
    glm::mat4 transformation;
    AbstractTransformable *parent;
    std::vector<std::shared_ptr<AbstractTransformable>> children;
    std::string name;
};

class Empty;
typedef std::shared_ptr<Empty> EmptyPtr;
class Empty : public AbstractTransformable, public MindTree::PyExposable
{
public:
    Empty() : AbstractTransformable(EMPTY) {}
    virtual ~Empty() {}
    AbstractTransformablePtr clone() const override
    {
        auto *obj = new Empty(*this);
        return std::shared_ptr<Empty>(obj);
    }

protected:
    Empty(const Empty &other) : AbstractTransformable(other) {}
};

class ObjectData : public MindTree::Object, public MindTree::PyExposable
{
public:
    enum eDataType {
        MESH
    };

    ObjectData(eDataType t);
    eDataType getType();
    void setType(eDataType type);
    virtual ~ObjectData();

private:
    eDataType type;
};

typedef std::shared_ptr<ObjectData> ObjectDataPtr;

class MeshData : public ObjectData
{
public:
    MeshData();
    virtual ~MeshData();
    std::string getName();

    void computeFaceNormals();
    void computeVertexNormals();

private:
    std::string name;
};
typedef std::shared_ptr<MeshData> MeshDataPtr;

class GeoObject;
typedef std::shared_ptr<GeoObject> GeoObjectPtr;
class GeoObject : public AbstractTransformable, public MindTree::PyExposable
{
public:
    GeoObject();
    ~GeoObject();

    AbstractTransformablePtr clone() const override;
    std::shared_ptr<ObjectData> getData();
    void setData(std::shared_ptr<ObjectData> value);

    MaterialInstancePtr getMaterial();
    void setMaterial(MaterialInstancePtr material);

protected:
    GeoObject(const GeoObject &other);

private:
    std::shared_ptr<ObjectData> data;
    MaterialInstancePtr _material;
};

class CreateGroupNode;
class Camera;
class GeoObject;
class Light;

class Group : public MindTree::Object, public MindTree::PyExposable
{
public:
    Group();
    Group(const Group &other) : members(other.members) {}
    virtual ~Group();
    void addMember(std::shared_ptr<AbstractTransformable> trans);
    std::vector<std::shared_ptr<AbstractTransformable>> getMembers()const;
    void addMembers(std::vector<std::shared_ptr<AbstractTransformable>> list);
    std::vector<std::shared_ptr<Camera>> getCameras();
    std::vector<std::shared_ptr<GeoObject>> getGeometry();
    std::vector<std::shared_ptr<Light>> getLights();

private:
    std::vector<std::shared_ptr<AbstractTransformable>> members;
};

typedef std::shared_ptr<Group> GroupPtr;

class Camera;
typedef std::shared_ptr<Camera> CameraPtr;
class Camera : public AbstractTransformable
{
public:
    Camera();
    virtual ~Camera();

    AbstractTransformablePtr clone() const override;

    void setProjection(double aspect);
    glm::mat4 getProjection();
    glm::mat4 getViewMatrix();

private:
    glm::mat4 projection;
    double fov;
    bool perspective;
    double near, far;
};

#endif /* end of include guard: OBJECT */
