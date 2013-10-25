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

#include "ray.h"

#include "iostream"

Ray::Ray(Vector start, Vector dir)
    : start(start), dir(dir)
{
}

Ray::~Ray()
{
}

bool Ray::intersectPlane(Vector orig, Vector v1, Vector v2, double *u, double *v, double *distance, Vector *hitpoint)    
{
    Vector diru = v1 - orig;
    Vector dirv = v2 - orig;

    Vector startorig = start - orig;
    Vector negdir = dir*-1;
    Vector v2crossnegdir = dirv.cross(negdir);

    double det = diru.dot(v2crossnegdir);
    double det1 = startorig.dot(v2crossnegdir);
    double det2 = diru.dot(startorig.cross(negdir));
    double det3 = diru.dot(dirv.cross(startorig));
    double tmpu, tmpv, tmpdist;
    tmpdist = det3/det;

    tmpu = det1/det;
    tmpv = det2/det;

    if(u) *u = tmpu;
    if(v) *v = tmpu;
    if(distance) *distance = tmpdist;
    if(hitpoint) {
        Vector tmp1 = diru * tmpu;
        Vector tmp2 = dirv * tmpv;
        *hitpoint = tmp1 + tmp2;
        *hitpoint = *hitpoint + orig;
    }
    return true;
}

//bool Ray::intersectTriangle()    
//{
//}
//
//bool Ray::intersectCube()    
//{
//}
//
//bool Ray::intersectSphere()    
//{
//}
