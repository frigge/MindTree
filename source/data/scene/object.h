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

#include "source/data/nodes/data_node.h"
#include "source/data/scene/cache_main.h"
#include "source/data/code_generator/glslwriter.h"
#include "QMatrix4x4"

typedef struct GLShaderCode
{
    enum shaderType {
        Fragment,
        Vertex,
        Geometry
    } type;
    GLShaderCode(QString code, shaderType type, std::vector<Parameter> parameters) : code(code), type(type), parameters(parameters) {};
    std::vector<Parameter> parameters;
    QString code;
} GLShaderCode;

template<class T, unsigned long chunksize>
struct DataChunk
{
    DataChunk() : next(0), data(new T[chunksize]) {};
    ~DataChunk() {delete [] data; if(next)delete next;};
    T* data;
    DataChunk *next;
};

template<class T, unsigned long chunksize>
class FRGDataList
{
public:
    FRGDataList() : size(0), firstChunk(new DataChunk<T, chunksize>()) {}
    virtual ~FRGDataList(){delete firstChunk;}
    DataChunk<T, chunksize>* first() {return firstChunk;}

    DataChunk<T, chunksize>* end() {
        DataChunk<T, chunksize> *f = firstChunk;
        while(f->next) f = f->next;
        return f;
    }

    void append(T d) {
        if(size == 0){
            firstChunk->data[0] = d;
            size++;
            return;
        }
        if(!(size % chunksize)) {
            DataChunk<T, chunksize> *e = end();
            e->next = new DataChunk<T, chunksize>();
            e->next->data[0] = d;
            size++;
            return;
        }
        DataChunk<T, chunksize> *e = end();
        e->data[size%chunksize] = d;
        size++;
    }
    unsigned long getSize(){return size;}

    T* toArray() {
        if(size==0)
            return 0;
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

typedef struct Polygon
{
    Polygon() : vertices(0), vertexcount(0) {};
    Polygon(const Polygon &poly){*this = poly;}
    ~Polygon() {if(vertices)delete [] vertices;};
    Polygon &operator=(const Polygon& poly)
    {
        vertexcount=poly.vertexcount;
        vertices = new int[vertexcount];
        for(int i=0; i<vertexcount; i++)vertices[i] = poly.vertices[i];
        return *this;
    }
    int *vertices;
    int vertexcount;
} Polygon;

class PolygonList : public FRGDataList<Polygon, 100>
{
public:
    PolygonList(){};
    unsigned int** getPolyIndices();
    unsigned int* getPolySizes();
};

class VertexList : public FRGDataList<Vector, 10000>
{
public:
    VertexList() {};
    double* to1DArray();
};

class Object
{
public:
    Object(const DNode *node);
    ~Object();
    QString getName();
    void setName(QString value);
    int getVertCnt()const;
    int getPolyCnt()const;
    Polygon* getPolygons();
    double* getVertices();
    unsigned int* getPolySizes();
    unsigned int** getRawPolyArray();
    PolygonList* getPList();
    VertexList* getVList();
    void setVertices(VertexList *l);
    void setPolygons(PolygonList *p);
    void setGLFragID(int ID);
    void setGLVertexID(int ID);
    void setGLGeoID(int ID);
    GLShaderCode* getGLFrag();
    GLShaderCode* getGLVertex();
    GLShaderCode* getGLGeo();
    void setTransformation(QMatrix4x4 value);
    QMatrix4x4 getTransformation();
    QMatrix4x4 getWorldTransformation();
    void setParent(Object *p);
    Object* getParent();
    void addChild(Object *obj);
    void addChildren(QList<Object*>objs);
    void removeChild(Object* child);
    QList<Object*> getChildren();
    void rmUser();
    void addUser();
    void makeInstance(Object* obj);
    Object* getOwner();

private:
    VertexList *vertices;
    PolygonList *polygons;
    QMatrix4x4 transformation;
    GLShaderCode *frag, *vert, *geo;
    QString name;
    QList<Object*> children;
    Object *parent;
    Object *owner;
    int usercount;
};

class PolygonNode : public DNode
{
public:
    PolygonNode(bool raw=false);
    PolygonNode(const PolygonNode *node);
};

class ObjectNode : public DNode
{
public:
    ObjectNode(bool raw=false);
    ObjectNode(const ObjectNode *node);
};

class TransformNode : public DNode
{
public:
    TransformNode(bool raw=false);
    TransformNode(const TransformNode *node);
};

#endif /* end of include guard: OBJECT */
