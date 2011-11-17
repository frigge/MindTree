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

Object::Object()
    : vertices(0), polygons(0), polycount(0), vertexcount(0)
{
}

Object::~Object()
{
    delete [] vertices;
    delete [] polygons;
}

int Object::getVertCnt()    
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

int Object::getPolyCnt()    
{
    return polycount;
}

void Object::appendVertices(Vector *verts, int size)    
{
    int oldcnt = vertexcount;
    vertexcount += size;
    Vector *newverts = new Vector[vertexcount];
    Vector *oldverts = vertices;
    Vector *vertstoappend = verts;
    int i = 0;
    if(oldverts)
        for(i=0; i<vertexcount; i++)
            newverts[i] = oldverts[i];

    for(i=vertexcount-size; i<vertexcount; i++)
        newverts[i] = vertstoappend[i-oldcnt];

    delete[] vertices;

    vertices = newverts;
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
        new DinSocket("Vertex Normals", VECTOR, this);
        new DinSocket("Polygons", POLYGON, this);
        new DinSocket("Polygon Normals", VECTOR, this);
        new DoutSocket("Object", SCENEOBJECT, this);
        setDynamicSocketsNode(IN);
    }
}

ObjectNode::ObjectNode(const ObjectNode *node)
    : DNode(node)
{
}
