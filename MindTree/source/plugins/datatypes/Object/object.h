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
//#include "source/data/code_generator/glslwriter.h"
#include "glm/glm.hpp"
#include "QMatrix4x4"
#include "data/python/pyexposable.h"
#include "data/properties.h"

template<class T, unsigned long chunksize>
struct DataChunk
{
    DataChunk() : next(nullptr), data(new T[chunksize]) {};
    ~DataChunk() {delete [] data; if(next)delete next;};
    T* data;
    DataChunk *next;
};

template<class T, unsigned long chunksize>
class FRGDataList
{
    class Iterator
    {
    public:
        Iterator(FRGDataList &list, int pos) : pos(pos), list(list) {}
        Iterator(const Iterator& other) : pos(other.pos), list(other.list) {}

        Iterator& operator++() {pos++; return *this;}
        T operator*() {return list[pos];}
        bool operator!=(const Iterator &other) {return other.pos != pos;}
    private:
        FRGDataList &list;
        int pos;
    };

public:
    FRGDataList() : size(0), firstChunk(new DataChunk<T, chunksize>()) {}
    virtual ~FRGDataList(){delete firstChunk;}
    DataChunk<T, chunksize>* first() {return firstChunk;}

    DataChunk<T, chunksize>* lastChunk() {
        DataChunk<T, chunksize> *f = firstChunk;
        while(f->next) f = f->next;
        return f;
    }

    Iterator begin(){
        return Iterator(*this, 0);
    }

    Iterator end(){
        return Iterator(*this, size);
    }

    virtual void append(T d) {
        if(size == 0){
            firstChunk->data[0] = d;
            size++;
            return;
        }
        if(!(size % chunksize)) {
            DataChunk<T, chunksize> *e = lastChunk();
            e->next = new DataChunk<T, chunksize>();
            e->next->data[0] = d;
            size++;
            return;
        }
        DataChunk<T, chunksize> *e = lastChunk();
        e->data[size%chunksize] = d;
        size++;
    }
    unsigned long getSize(){return size;}

    T* toArray() {
        if(size==0)
            return nullptr;
        T* array = new T[size];
        DataChunk<T, chunksize> *f = firstChunk;
        for(int i=0; i<size; i++){
            array[i] = f->data[i%chunksize];
            if(!((i+1)%chunksize)) f = f->next;
        }
        return array;
    }

    void resize(unsigned int newsize) {
        int curr_chunk_nr = size/chunksize;
        int new_chunk_nr = newsize/chunksize;
        int ch_ind = size%chunksize;
        size = newsize;
        DataChunk<T, chunksize> *f = firstChunk;
        for(int i=0; i < new_chunk_nr; i++) {
            if(i>= curr_chunk_nr)
                f->next = new DataChunk<T, chunksize>();
            f = f->next;
        }
    }

    T& operator[](unsigned long i) {
        if(i >= size)
            resize(i+1);
        int chunknr = i/chunksize;
        int chunk_i = i%chunksize;
        DataChunk<T, chunksize> *f = first();
        for(int j=0; j!=chunknr; j++, f=f->next);
        return f->data[chunk_i];
    }

protected:
    DataChunk<T, chunksize> *firstChunk;

private:
    unsigned long size;
};

class Polygon
{
public:
    Polygon() {}
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

typedef std::vector<std::vector<int> > intVecList_t;
class PolygonList : public FRGDataList<Polygon, 100>
{
public:
    PolygonList(){};
    unsigned int** getPolyIndices();
    unsigned int* getPolySizes();
    virtual void append(Polygon d);
    intVecList_t& getVertexMap();
private:
    intVecList_t vertex_map;
};

class VectorList : public FRGDataList<glm::vec3, 10000>
{
public:
    VectorList() {};
    float* to1DArray();
};

typedef VectorList VertexList;

class CustomShader
{
public:
    CustomShader();
    virtual ~CustomShader();

private:
    /* data */
};

class QGLBuffer;
class QGLShaderProgram;
class MeshData;
class AbstractTransformableNode;
class AbstractTransformable : public std::enable_shared_from_this<AbstractTransformable>
{
public:
    enum eObjType {
        GEO, CAMERA, LIGHT
    };
    AbstractTransformable(eObjType t);
    virtual ~AbstractTransformable();
    AbstractTransformable::eObjType getType();
    std::string getName();
    void setName(std::string value);
    //void setTransformation(QMatrix4x4 value);

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

    void setParent(std::shared_ptr<AbstractTransformable> p);
    std::shared_ptr<AbstractTransformable> getParent();
    void addChild(std::shared_ptr<AbstractTransformable> obj);
    void addChildren(std::list<std::shared_ptr<AbstractTransformable>>objs);
    void removeChild(std::shared_ptr<AbstractTransformable> child);
    std::list<std::shared_ptr<AbstractTransformable>> getChildren();

private:
    eObjType type;
    glm::vec3 center;
    glm::mat4 transformation;
    std::shared_ptr<AbstractTransformable> parent;
    std::list<std::shared_ptr<AbstractTransformable>> children;
    std::string name;
};

class ObjectData : public MindTree::PyExposable
{
public:
    enum eDataType {
        MESH, SPLINE, NURBS, PATCH
    };
    ObjectData(eDataType t);
    eDataType getType();
    void setType(eDataType type);
    virtual ~ObjectData();

private:
    eDataType type;
};

class MeshData : public ObjectData
{
public:
    MeshData();
    virtual ~MeshData();
    std::string getName();

    void addProperty(std::string name, MindTree::Property prop);
    template<typename T>
    T getProperty(std::string name) {
        try {
            return properties.at(name).getData<T>();
        } catch (const std::out_of_range &e){
            std::cout << "no Property of name "<< name << " stored" << std::endl;
            return T();
        }
    }

    MindTree::PropertyMap& getPropertyMap();

    void computeFaceNormals();
    void computeVertexNormals();
    const MindTree::DNode* getNode();

private:
    MindTree::PropertyMap properties;
    std::string name;
};

class Object : public AbstractTransformable, public MindTree::PyExposable
{
public:
    Object();
    ~Object();
    void makeInstance(std::shared_ptr<Object> obj);
    std::shared_ptr<ObjectData> getData();
    void setData(std::shared_ptr<ObjectData> value);
    void addProperty(std::string name, MindTree::Property prop);
    template<typename T>
    T getProperty(std::string name) {
        try {
            return properties.at(name).getData<T>();
        } catch (const std::out_of_range &e){
            std::cout << "no Property of name "<< name << " stored" << std::endl;
            return T();
        }
    }

    const MindTree::PropertyMap& getPropertyMap();

private:
    std::shared_ptr<ObjectData> data;
    MindTree::PropertyMap properties;
};

class CreateGroupNode;
class Camera;
class Object;
class Light;

class Group : public MindTree::PyExposable
{
public:
    Group();
    Group(const Group &grp);
    virtual ~Group();
    void addMember(std::shared_ptr<AbstractTransformable> trans);
    std::list<std::shared_ptr<AbstractTransformable>> getMembers()const;
    void addMembers(std::list<std::shared_ptr<AbstractTransformable>> list);
    std::list<std::shared_ptr<Camera>> getCameras();
    std::list<std::shared_ptr<Object>> getGeometry();
    std::list<std::shared_ptr<Light>> getLights();

private:
    std::list<std::shared_ptr<AbstractTransformable>> members;
};

class Camera : public AbstractTransformable
{
public:
    Camera();
    void setProjection(double aspect);
    glm::mat4 getProjection();
    glm::mat4 getViewMatrix();
    virtual ~Camera();

private:
    glm::mat4 projection;
    double fov;
    bool perspective;
    double near, far;
};

class Light : public AbstractTransformable
{
public:
    Light();
    virtual ~Light();

private:
    double intensity;
};

class Scene
{
public:
    Scene();
    virtual ~Scene();
    std::string getName();
    void setName(std::string n);
    const MindTree::DNode* getNode();
    std::list<Camera*> getCameras();
    std::list<Light*> getLights();
    std::list<Object*> getObjects();
    void setObjects(std::list<Object*> objs);
    void setLights(std::list<Light*> lights);
    void setCameras(std::list<Camera*> cams);

private:
    std::string name;
    std::list<Object*> objects;
    std::list<Light*>lights;
    std::list<Camera*>cameras;
};

class PolygonNode : public MindTree::DNode
{
public:
    PolygonNode(bool raw=false);
    PolygonNode(const PolygonNode &node);
};

class ObjectDataNodeBase : public MindTree::DNode
{
public:
    ObjectDataNodeBase(std::string name);
    ObjectDataNodeBase(const ObjectDataNodeBase &node);
    virtual ~ObjectDataNodeBase();
    virtual std::shared_ptr<ObjectData> getObjectData();

protected:
    void setObjectData(std::shared_ptr<ObjectData> d);

private:
    std::shared_ptr<ObjectData> data;
};

class ComposeMeshNode : public ObjectDataNodeBase
{
public:
    ComposeMeshNode(bool raw=false);
    ComposeMeshNode(const ComposeMeshNode &node);
    MindTree::DAInSocket* getVertSocket();
    MindTree::DAInSocket* getPolySocket();
    //std::shared_ptr<MeshData> getObjectData();

private:
    MindTree::DAInSocket *vertSocket, *polySocket;
};

class AbstractTransformableNode : public MindTree::DNode
{
public:
    AbstractTransformableNode(std::string, bool raw=false);
    AbstractTransformableNode(const AbstractTransformableNode &node);
    virtual ~AbstractTransformableNode();

    virtual std::shared_ptr<AbstractTransformable> getObject();
    void setObject(std::shared_ptr<AbstractTransformable> obj);

    void setNodeName(std::string name);
    
    static bool isTransformable(MindTree::DNode *node);

    MindTree::DinSocket* getTransformSocket();
    MindTree::DinSocket* getRotateSocket();
    MindTree::DinSocket* getScaleSocket();

private:
    std::shared_ptr<AbstractTransformable> transformable;
   MindTree::DinSocket *transSocket, *rotSocket, *scaleSocket;
};

class ObjectNode : public AbstractTransformableNode
{
public:
    ObjectNode(bool raw=false);
    ObjectNode(const ObjectNode &node);
    //Object* getObject();
    MindTree::DinSocket* getObjDataSocket();
    MindTree::DinSocket* getGeoSocket();
    MindTree::DinSocket* getVertSocket();
    MindTree::DinSocket* getFragSocket();

private:
    MindTree::DinSocket* objDataSocket, *fragSocket, *vertSocket, *geoSocket; 
};

class CreateGroupNode : public MindTree::DNode
{
public:
    CreateGroupNode(std::string name="Group", bool raw=false);
    CreateGroupNode(const CreateGroupNode &node);
    virtual ~CreateGroupNode();
    Group* getGroup();

private:
    Group *group;
};

class CameraNode : public AbstractTransformableNode
{
public:
    CameraNode(bool raw=false);
    CameraNode(const CameraNode &node);
    //Camera* getObject();
};

class LightNode : public AbstractTransformableNode
{
public:
    enum LightType {POINTLIGHT, SPOTLIGHT, DISTANTLIGHT};
    LightNode(std::string name, bool raw=false);
    LightNode(const LightNode &node);

    //Light* getObject();
    LightType getLightType();
    void setLightType(LightType t);

private:
    LightType lighttype;
};

class PointLightNode : public LightNode
{
public:
    PointLightNode(bool raw=false);
    PointLightNode(const PointLightNode &node);
};

class SpotLightNode : public LightNode
{
public:
    SpotLightNode(bool raw=false);
    SpotLightNode(const SpotLightNode &node);
};

class DistantLightNode : public LightNode
{
public:
    DistantLightNode(bool raw=false);
    DistantLightNode(const DistantLightNode &node);
};

#endif /* end of include guard: OBJECT */
