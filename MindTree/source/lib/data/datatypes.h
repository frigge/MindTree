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

template<class T>
class Vec2
{
    public:
        Vec2 () : _x(0), _y(0) {}

        Vec2(T x, T y) : _x(x), _y(y) {}

        const T& x() { return _x; }
        const T& y() { return _y; }

        Vec2& operator+= (const Vec2 other) { _x += other._x; _y += other._y; return *this; }
        Vec2& operator-= (const Vec2 other) { _x -= other._x; _y -= other._y; return *this; }

        Vec2& operator + (const Vec2 other) { return Vec2<T>(_x + other._x, _y + other._y); }
        Vec2& operator - (const Vec2 other) { return Vec2<T>(_x + other._x, _y + other._y); }

    private:
        T _x, _y;
};

typedef Vec2<float> Vec2f;
typedef Vec2<double> Vec2d;
typedef Vec2<int> Vec2i;

//template<typename T>
//class Vec3
//{
//public:
//    Vec3d();
//    Vec3d(double x, double y, double z);
//    Vec3d(const Vec3d &vec);
//    Vec3d cross(Vec3d &b);
//    double dot(Vec3d b);
//    double len();
//    Vec3d normalized();
//    void normalize();
//    double x;
//    double y;
//    double z;
//    Vec3d& operator+=(const Vec3d &other);
//    Vec3d& operator*=(double d);
//Vec3& operator+(const Vec3d &b)
//{
//    return Vec3d(a.x + b.x, a.y + b.y, a.z + b.z); 
//}
//
//Vec3& operator-(const Vec3d &b)
//{
//    return Vec3d(a.x - b.x, a.y - b.y, a.z - b.z);
//}
//
//Vec3& operator*(const Vec3d &vec, double d)
//{
//    return Vec3d(vec.x*d, vec.y*d, vec.z*d);
//}
//
//Vec3& operator+=(const Vec3 &other)
//{
//    x += other.x;
//    y += other.y;
//    z += other.z;
//    return *this;
//}
//
//Vec3& operator*=(double d)    
//{
//    x *= d;
//    y *= d;
//    z *= d;
//    return *this;
//}
//};
//
//typedef Vec3d Vector;
//
#endif /* end of include guard: DATATYPES_EVYG6DVA */
