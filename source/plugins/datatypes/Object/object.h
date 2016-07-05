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

#include "data/mtobject.h"

#define GLM_SWIZZLE
#include "glm/glm.hpp"

#include "data/python/pyexposable.h"
#include "data/properties.h"
#include "material.h"

#include "mutex"
#include "atomic"

typedef std::vector<glm::vec3> VertexList;
typedef std::shared_ptr<VertexList> VertexListPtr;
struct Polygon : public std::vector<uint> {
    using std::vector<uint>::vector;
};
typedef std::vector<Polygon> PolygonList;
typedef std::shared_ptr<PolygonList> PolygonListPtr;

class MeshData;
class AbstractTransformable;
typedef std::shared_ptr<AbstractTransformable> AbstractTransformablePtr;
class AbstractTransformable : public MindTree::Object
{
public:
    enum eObjType {
        GEO, CAMERA, LIGHT, EMPTY, JOINT
    };
    AbstractTransformable(eObjType t);
    virtual ~AbstractTransformable();

    virtual int getVertexCount() const;
    virtual int getPolygonCount() const;

    virtual AbstractTransformablePtr clone() const;

    AbstractTransformable::eObjType getType();
    std::string getName();
    void setName(std::string value);

    glm::mat4 getTransformation();
    void setTransformation(glm::mat4 value);
    glm::mat4 getWorldTransformation() const;
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

    AbstractTransformable* getParent();
    const AbstractTransformable* getParent() const;
    void addChild(std::shared_ptr<AbstractTransformable> child);
    void addChildren(std::vector<AbstractTransformablePtr> objs);
    void removeChild(AbstractTransformable *child);
    std::vector<std::shared_ptr<AbstractTransformable>> getChildren();
    std::vector<std::shared_ptr<AbstractTransformable>> getChildren() const;

    void setProperty(std::string name, MindTree::Property prop);

protected:
    AbstractTransformable(const AbstractTransformable &other);

private:
    eObjType type;
    glm::vec3 center;
    glm::mat4 transformation;

    //cached for performance on deep hierarchies (skeletons)
    glm::mat4 worldTransform_;
    mutable std::mutex _transformationLock;
    std::mutex _centerLock;

    AbstractTransformable *_parent;
    std::vector<std::shared_ptr<AbstractTransformable>> _children;
    std::string _name;
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
        MESH,
        POINTCLOUD
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
class Edge {
    uint v0_, v1_;
public:
    Edge(uint v0, uint v1) :
        v0_{v0}, v1_{v1}
    {}

    uint v0() const { return v0_; }
    uint v1() const { return v1_; }

    bool operator==(const Edge &edge) const
    {
        return (edge.v0_ == v0_ && edge.v1_ == v1_)
        || (edge.v0_ == v1_ && edge.v1_ == v0_);
    }
};
    MeshData();
    virtual ~MeshData();
    std::string getName();

    void computeVertexNormals();
    int getVertexCount() const;
    int getPolygonCount() const;

private:
    std::string name;
};
typedef std::shared_ptr<MeshData> MeshDataPtr;

class PointCloud : public ObjectData
{
    PointCloud() : ObjectData(POINTCLOUD) {}
};

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

    int getVertexCount() const override;
    int getPolygonCount() const override;

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
    std::vector<std::shared_ptr<Camera>> getCameras() const;
    std::vector<std::shared_ptr<GeoObject>> getGeometry() const;
    std::vector<std::shared_ptr<Light>> getLights() const;
    void setProperty(std::string name, MindTree::Property prop);

    int getVertexCount() const;
    int getPolygonCount() const;

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

    void setAspect(double aspect);
    void setNear(double near);
    void setFar(double far);
    void setFov(double fov);

    void setResolution(int width, int height);
    int getWidth() const;
    int getHeight() const;

private:
    Camera(const Camera &other);

    std::atomic<float> _fov;
    std::atomic<float> _aspect;
    std::atomic<float> _near, _far;

    std::atomic<int> _width, _height;
};

namespace std {
    template<>
    struct hash<MeshData::Edge> {
        typedef size_t result_type;
        typedef MeshData::Edge argument_type;

        result_type operator()(argument_type const &value) const
        {
            size_t lhs, rhs;
            if(value.v0() < value.v1()) {
                lhs = value.v0();
                rhs = value.v1();
            }
            else {
                lhs = value.v1();
                rhs = value.v0();
            }
            return lhs^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
        }
    };
}
#endif /* end of include guard: OBJECT */
