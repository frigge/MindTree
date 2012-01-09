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

#include "object.h"

#include "source/data/base/frg.h"
#include "source/data/base/project.h"
#include "source/data/base/properties.h"

double* VertexList::to1DArray()    
{
    if(getSize()==0)
        return 0;

    double *arr = new double[getSize() * 3];
    DataChunk<Vector, 10000> *f = firstChunk;
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
        return 0;

    unsigned int *arr = new unsigned int[getSize()];
    DataChunk<Polygon, 100> *f = firstChunk;
    for(int i=0; i<getSize(); i++) {
        arr[i] = f->data[i%100].vertexcount;
        if(!(i+1)%100) f = f->next;
    }
    return arr;
}

unsigned int** PolygonList::getPolyIndices()    
{
    unsigned int **indices = new unsigned int*[getSize()];
    DataChunk<Polygon, 100> *f = firstChunk;
    for(int i=0; i<getSize(); i++) {
        indices[i] = new unsigned int[f->data[i%100].vertexcount];
        for(int j=0; j<f->data[i%100].vertexcount; j++)
            indices[i][j] = f->data[i%100].vertices[j];
        if(!(i+1)%100) f = f->next;
    }
    return indices;
}

Object::Object(const DNode *node)
    : vertices(0), polygons(0),
    frag(0), vert(0), geo(0), parent(0), owner(0),
    usercount(1)
{
    name = node->getNodeName();
    FRG::CurrentProject->cacheObject(this, node);
}

Object::~Object()
{
    if(vertices && !owner && usercount == 1)
        delete vertices;
    if(polygons && !owner && usercount == 1)
        delete polygons;
    
    if(parent) parent->removeChild(this);
    if(owner) owner->rmUser();

    FRG::CurrentProject->removeObject(this);
}

Object* Object::getOwner()    
{
    return owner;
}

void Object::makeInstance(Object* obj)    
{
    owner = obj;
    owner->addUser();
}

VertexList* Object::getVList()    
{
    if(owner) return owner->getVList();
    return vertices;
}

PolygonList* Object::getPList()    
{
    if(owner) return owner->getPList();
    return polygons;
}

void Object::addUser()    
{
    usercount++;
}

void Object::rmUser()    
{
    usercount--;
}

void Object::removeChild(Object* child)    
{
    children.removeAll(child);
}

void Object::setParent(Object *p)    
{
    parent = p;
    parent->addChild(this);
}

Object* Object::getParent()    
{
    return parent;
}

void Object::addChild(Object *obj)    
{
    children.append(obj);
}

void Object::addChildren(QList<Object*>objs)    
{
    children.append(objs);
}

QList<Object*> Object::getChildren()    
{
    return children; 
}

QString Object::getName()
{
    return name;
}

void Object::setName(QString value)
{
    name = value;
}

int Object::getVertCnt()    const
{
    if(owner) return owner->getVertCnt();
    if(!vertices)return 0;
    return vertices->getSize();
}

double* Object::getVertices()    
{
    if(owner)return owner->getVertices();
    if(!vertices)return 0;
    return vertices->to1DArray();
}

Polygon* Object::getPolygons()    
{
    if(owner) return owner->getPolygons();
    if(!polygons) return 0;
    return polygons->toArray(); 
}

unsigned int** Object::getRawPolyArray()    
{
    if(owner) return owner->getRawPolyArray();
    return polygons->getPolyIndices();
}

unsigned int* Object::getPolySizes()    
{
    if(owner)return owner->getPolySizes();
    return polygons->getPolySizes(); 
}

int Object::getPolyCnt()   const 
{
    if(owner) return owner->getPolyCnt();
    if(!polygons) return 0;
    return polygons->getSize();
}

void Object::setVertices(VertexList *l)    
{
    if(owner) owner->rmUser();
    owner=0;
    if(vertices) delete vertices;
    vertices = l; 
}

void Object::setPolygons(PolygonList *p)    
{
    if(owner) owner->rmUser();
    owner=0;
    if(polygons) delete polygons;
    polygons = p; 
}

void Object::setGLFragID(int ID)    
{
    frag = FRG::CurrentProject->getGLSLShader(ID);
}

void Object::setGLVertexID(int ID)    
{
    vert = FRG::CurrentProject->getGLSLShader(ID);
}

void Object::setGLGeoID(int ID)    
{
    geo = FRG::CurrentProject->getGLSLShader(ID);
}

QMatrix4x4 Object::getTransformation()
{
    return transformation;
}

void Object::setTransformation(QMatrix4x4 value)
{
    transformation = value;
}

QMatrix4x4 Object::getWorldTransformation()    
{
    QMatrix4x4 trans = transformation;
    Object *p = parent;
    while(p){
        trans = p->getTransformation() * trans;
        p = p->getParent();
    }
    return trans;
}

GLShaderCode* Object::getGLFrag()    
{
    return frag;
}

GLShaderCode* Object::getGLVertex()    
{
    return vert;
}

GLShaderCode* Object::getGLGeo()    
{
    return geo;
}

PolygonNode::PolygonNode(bool raw)
    : DNode("Object")
{
    setNodeType(POLYGONNODE);
    if(!raw){
        new DoutSocket("Polygon", POLYGON, this);
        setDynamicSocketsNode(IN);
    }
}

PolygonNode::PolygonNode(const PolygonNode *node)
    : DNode(node)
{
}

ObjectNode::ObjectNode(bool raw)
    : DNode("Object")
{
    setNodeType(OBJECTNODE);
    if(!raw){
        new DinSocket("Vertices", VECTOR, this);
        new DinSocket("Polygons", POLYGON, this);
        new DinSocket("GLSL Fragment Shader", INTEGER, this);
        new DinSocket("GLSL Vertex Shader", INTEGER, this);
        new DinSocket("GLSL Geometry Shader", INTEGER, this);
        new DoutSocket("Object", SCENEOBJECT, this);
        setDynamicSocketsNode(IN);
    }
}

ObjectNode::ObjectNode(const ObjectNode *node)
    : DNode(node)
{
}

TransformNode::TransformNode(bool raw)
    : DNode("Transform")
{
    setNodeType(TRANSFORM);
    if(!raw){
        new DinSocket("Object", SCENEOBJECT, this);
        new DinSocket("Translate", VECTOR, this);
        new DinSocket("Rotate", VECTOR, this);
        DinSocket *scaleSocket = new DinSocket("Scale", VECTOR, this);
        ((VectorProperty*)scaleSocket->getProperty())->setValue(Vector(1, 1, 1));
        new DoutSocket("Object", SCENEOBJECT, this);
    }
}

TransformNode::TransformNode(const TransformNode *node)
    : DNode(node)
{
}
