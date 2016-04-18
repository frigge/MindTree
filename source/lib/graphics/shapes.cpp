#include "iostream"
#include "shapes.h"

Plane::Plane()
{
}

Plane::Plane(const glm::vec3 &point, const glm::vec3 &normal) :
    _point(point), _normal(normal)
{
}

Plane Plane::operator*(const glm::mat4 &mat) const
{
    return Plane((mat * glm::vec4(_point, 1)).xyz(),
                 (mat * glm::vec4(_normal, 0)).xyz());
}

glm::vec3 Plane::point() const
{
    return _point;
}

glm::vec3 Plane::normal() const
{
    return _normal;
}

void Plane::setPoint(const glm::vec3 &point)
{
    _point = point;
}

void Plane::setNormal(const glm::vec3 &normal)
{
    _normal = normal;
}

Rectangle::Rectangle()
{
}

Rectangle::Rectangle(const glm::vec3 &bl,
                     const glm::vec3 &v1,
                     const glm::vec3 &v2) :
    _bottomLeft(bl),
    _v1(v1),
    _v2(v2)
{
}

Rectangle Rectangle::operator*(const glm::mat4 &mat) const
{
    return Rectangle((mat * glm::vec4(_bottomLeft, 1)).xyz(),
                     (mat * glm::vec4(_v1, 0)).xyz(),
                     (mat * glm::vec4(_v2, 0)).xyz());
}

glm::vec3 Rectangle::bottomLeft() const
{
    return _bottomLeft;
}

glm::vec3 Rectangle::topRight() const
{
    return _bottomLeft + _v1 + _v2;
}

glm::vec3 Rectangle::normal() const
{
    glm::vec3 n = glm::cross(_v1, _v2);
    return glm::cross(_v1, _v2);
}

float Rectangle::diagonal() const
{
    return glm::length(topRight() - _bottomLeft);
}

glm::vec3 Rectangle::uvector() const
{
    return _v1;
}

glm::vec3 Rectangle::vvector() const
{
    return _v2;
}

void Rectangle::setBottomLeft(const glm::vec3 &bl)
{
    _bottomLeft = bl;
}

Sphere::Sphere() :
    _radius(0)
{
}

Sphere::Sphere(const glm::vec3 &point, float radius) :
    _point(point), _radius(radius)
{
}

Sphere Sphere::operator*(const glm::mat4 &mat) const
{
    return Sphere((mat * glm::vec4(_point, 1)).xyz(), _radius);
}

glm::vec3 Sphere::point() const
{
    return _point;
}

float Sphere::radius() const
{
    return _radius;
}

void Sphere::setPoint(const glm::vec3 &point)
{
    _point = point;
}

void Sphere::setRadius(float radius)
{
    _radius = radius;
}

Box::Box()
{
}

Box::Box(const glm::vec3 &point, float width, float height, float depth) :
    _point(point),
    _width(width),
    _height(height),
    _depth(depth)
{
}

Box Box::operator*(const glm::mat4 &mat) const
{
    return Box((mat * glm::vec4(_point, 1)).xyz(),
               _width,
               _height,
               _depth);
}

glm::vec3 Box::point() const
{
    return _point;
}

float Box::width() const
{
    return _width;
}

float Box::height() const
{
    return _height;
}

float Box::depth() const
{
    return _depth;
}

void Box::setPoint(const glm::vec3 &point)
{
    _point = point;
}

void Box::setWidth(float width)
{
    _width = width;
}

void Box::setHeight(float height)
{
    _height = height;
}

void Box::setDepth(float depth)
{
    _depth = depth;
}
