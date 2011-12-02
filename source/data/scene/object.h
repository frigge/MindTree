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

typedef struct Polygon
{
    int *vertices;
    int vertexcount;
} Polygon;

class Object
{
public:
    Object();
    ~Object();
    int getVertCnt()const;
    int getPolyCnt()const;
    Polygon* getPolygons();
    Vector* getVertices();
    void appendPolygons(Polygon *polys, int size);
    void appendVertices(Vector *verts, int size);

private:
    Vector *vertices;
    Polygon *polygons;
    int polycount, vertexcount;
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

#endif /* end of include guard: OBJECT */
