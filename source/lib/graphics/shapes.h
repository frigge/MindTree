#ifndef MT_SHAPES
#define MT_SHAPES

#define GLM_SWIZZLE
#include "glm/glm.hpp"

class Plane
{
public:
    Plane();
    Plane(const glm::vec3 &point, const glm::vec3 &normal);

    Plane operator*(const glm::mat4 &mat) const;

    glm::vec3 point() const;
    glm::vec3 normal() const;
    void setPoint(const glm::vec3 &point);
    void setNormal(const glm::vec3 &normal);

private:
    glm::vec3 _point, _normal;
};

class Rectangle
{
public:
    Rectangle();
    Rectangle(const glm::vec3 &bl, const glm::vec3 &tr, const glm::vec3 &normal);

    Rectangle operator*(const glm::mat4 &mat) const;

    glm::vec3 bottomLeft() const;
    glm::vec3 topRight() const;
    glm::vec3 normal() const;
    float diagonal() const;
    glm::vec3 uvector() const;
    glm::vec3 vvector() const;

    void setBottomLeft(const glm::vec3 &bl);

private:
    glm::vec3 _bottomLeft, _v1, _v2;
};

class Sphere
{
public:
    Sphere();
    Sphere(const glm::vec3 &point, float radius);

    Sphere operator*(const glm::mat4 &mat) const;

    glm::vec3 point() const;
    float radius() const;
    void setPoint(const glm::vec3 &point);
    void setRadius(float radius);

private:
    glm::vec3 _point;
    float _radius;
};

class Box
{
public:
    Box();
    Box(const glm::vec3 &point, float width, float height, float depth);

    Box operator*(const glm::mat4 &mat) const;

    glm::vec3 point() const;
    float width() const;
    float height() const;
    float depth() const;
    void setPoint(const glm::vec3 &point);
    void setWidth(float width);
    void setHeight(float height);
    void setDepth(float depth);

private:
    glm::vec3 _point;
    float _width, _height, _depth;
};
#endif
