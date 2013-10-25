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

#ifndef DATATYPES_EVYG6DVA

#define DATATYPES_EVYG6DVA

class Vec3d
{
public:
    Vec3d();
    Vec3d(double x, double y, double z);
    Vec3d(const Vec3d &vec);
    Vec3d cross(Vec3d &b);
    double dot(Vec3d b);
    double len();
    Vec3d normalized();
    void normalize();
    double x;
    double y;
    double z;
    Vec3d& operator+=(const Vec3d &other);
    Vec3d& operator*=(double d);
};
Vec3d operator+(Vec3d &a, Vec3d &b);
Vec3d operator-(Vec3d &a, Vec3d &b);
Vec3d operator*(Vec3d &vec, double d);
Vec3d operator*(double d, Vec3d &vec);

typedef Vec3d Vector;

#endif /* end of include guard: DATATYPES_EVYG6DVA */
