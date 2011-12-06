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

Object::Object()
    : vertices(0), polygons(0), polycount(0), vertexcount(0),
    frag(0), vert(0), geo(0)
{
}

Object::~Object()
{
    if(vertices)
        delete [] vertices;
    if(polygons)
        delete [] polygons;
}

int Object::getVertCnt()    const
{
    return vertexcount;
}

Vector* Object::getVertices()    
{
    return vertices;
}

Polygon* Object::getPolygons()    
{
    return polygons; 
}

int Object::getPolyCnt()   const 
{
    return polycount;
}

void Object::appendVertices(Vector *verts, int size)    
{
    if(!verts)
        return;
    
    int oldcnt = vertexcount;
    vertexcount += size;
    if(vertices) {
        Vector *newverts = new Vector[vertexcount];
        Vector *oldverts = vertices;
        Vector *vertstoappend = verts;
        int i = 0;
        if(oldverts)
            for(i=0; i<vertexcount; i++)
                newverts[i] = oldverts[i];

        for(i=vertexcount-size; i<vertexcount; i++)
            newverts[i] = vertstoappend[i-oldcnt];

        if(vertices)
            delete[] vertices;

        vertices = newverts;
    }
    else{
        vertices = verts;
    }
}

void Object::appendPolygons(Polygon *polys, int size)    
{
    int newcnt = polycount + size;
    Polygon *newpolys = new Polygon[newcnt];

    int i = 0;
    for(i=0; i<polycount; i++)
        newpolys[i] = polygons[i];

    for(i=polycount; i<polycount+size; i++)
        newpolys[i] = polys[i-polycount];
   
   delete [] polygons;
   polygons = newpolys; 

   polycount += size;
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
