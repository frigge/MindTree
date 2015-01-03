#include "iostream"
#include "shapes.h"

Shape::Shape(Shape::Type type)
    : _type(type)
{
}

Shape::Type Shape::type() const
{
    return _type;
}

Plane::Plane()
    : Shape(PLANE)
{
}

Plane::Plane(glm::vec3 point, glm::vec3 normal)
    : Shape(PLANE), _point(point), _normal(normal)
{
}

glm::vec3 Plane::point() const
{
    return _point;
}

glm::vec3 Plane::normal() const
{
    return _normal;
}

void Plane::setPoint(glm::vec3 point)
{
    _point = point;
}

void Plane::setNormal(glm::vec3 normal)
{
    _normal = normal;
}

Rectangle::Rectangle()
    : Shape(RECTANGLE)
{
}

Rectangle::Rectangle(glm::vec3 bl, glm::vec3 v1, glm::vec3 v2)
    : Shape(RECTANGLE), _bottomLeft(bl), _v1(v1), _v2(v2)
{
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

void Rectangle::setBottomLeft(glm::vec3 bl)
{
    _bottomLeft = bl;
}

Sphere::Sphere()
    : Shape(SPHERE), _radius(0)
{
}

Sphere::Sphere(glm::vec3 point, float radius)
    : Shape(SPHERE), _point(point), _radius(radius)
{
}

glm::vec3 Sphere::point() const
{
    return _point;
}

float Sphere::radius() const
{
    return _radius;
}

void Sphere::setPoint(glm::vec3 point)
{
    _point = point;
}

void Sphere::setRadius(float radius)
{
    _radius = radius;
}

Box::Box()
    : Shape(BOX), _width(0), _height(0), _depth(0)
{
}

Box::Box(glm::vec3 point, float width, float height, float depth)
    : Shape(BOX), _point(point), _width(width), _height(height), _depth(depth)
{
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

void Box::setPoint(glm::vec3 point)
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

