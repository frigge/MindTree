#ifndef MT_SHAPES
#define MT_SHAPES

#include "glm/glm.hpp"

class Shape 
{
public:
    enum Type {
        PLANE,
        RECTANGLE,
        SPHERE,
        BOX,
        NONE
    };

    Shape(Shape::Type type=NONE);
    Type type() const;

private:
    Type _type;

};

class Plane : public Shape 
{
public:
    Plane();
    Plane(glm::vec3 point, glm::vec3 normal);

    glm::vec3 point() const;
    glm::vec3 normal() const;
    void setPoint(glm::vec3 point);
    void setNormal(glm::vec3 normal);

private:
    glm::vec3 _point, _normal;
};

class Rectangle : public Shape 
{
public:
    Rectangle();
    Rectangle(glm::vec3 bl, glm::vec3 tr, glm::vec3 normal);

    glm::vec3 bottomLeft() const;
    glm::vec3 topRight() const;
    glm::vec3 normal() const;
    float diagonal() const;
    glm::vec3 uvector() const;
    glm::vec3 vvector() const;

    void setBottomLeft(glm::vec3 bl);

private:
    glm::vec3 _bottomLeft, _v1, _v2;
};

class Sphere : public Shape 
{
public:
    Sphere();
    Sphere(glm::vec3 point, float radius);

    glm::vec3 point() const;
    float radius() const;
    void setPoint(glm::vec3 point);
    void setRadius(float radius);

private:
    glm::vec3 _point;
    float _radius;
};

class Box : public Shape 
{
public:
    Box();
    Box(glm::vec3 _point, float width, float height, float depth);

    glm::vec3 point() const;
    float width() const;
    float height() const;
    float depth() const;
    void setPoint(glm::vec3 point);
    void setWidth(float width);
    void setHeight(float height);
    void setDepth(float depth);


private:
    glm::vec3 _point;
    float _width, _height, _depth;
};
#endif
