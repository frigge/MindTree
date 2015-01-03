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

#include "datatypes.h"

//Vec3d:: Vec3d() 
//    : x(0), y(0), z(0)
//{
//}
//
//Vec3d::Vec3d(double x, double y, double z)
//    : x(x), y(y), z(z) 
//{
//}
//
//Vec3d::Vec3d(const Vec3d &vec) 
//    : x(vec.x), y(vec.y), z(vec.z)
//{
//}
//
//Vec3d Vec3d::cross(Vec3d &b)    
//{
//    return Vec3d(y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x); 
//}
//
//double Vec3d::dot(Vec3d b)    
//{
//    return x*b.x+y*b.y+z*b.z;
//}
//
//double Vec3d::len()    
//{
//    return sqrt(dot(*this));
//}
//
//Vec3d Vec3d::normalized()    
//{
//    return Vec3d(x/len(), y/len(), z/len());
//}
//
//void Vec3d::normalize()    
//{
//    double l = len();
//    x /= l;
//    y /= l;
//    z /= l;
//}
//
