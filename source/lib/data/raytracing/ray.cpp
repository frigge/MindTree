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


#define GLM_SWIZZLE
#include "iostream"
#include "glm/gtc/matrix_transform.hpp"
#include "ray.h"

Ray::Ray(glm::vec3 start, glm::vec3 d)
    : start(start), dir(d)
{
}

Ray::~Ray()
{
}

bool Ray::intersectPlane(const Plane &plane, glm::vec3 *hitpoint) const
{
    glm::vec3 v1(1, 0, 0);
    glm::vec3 v2(0, 1, 0);

    glm::vec3 rotvec;

    float angle1 = glm::dot(v1, glm::normalize(plane.normal()));
    float angle2 = glm::dot(v2, glm::normalize(plane.normal()));

    float angle;
    if(angle1 > angle2) {
        rotvec = glm::cross(v1, plane.normal());
        angle = angle1;
    }
    else {
        rotvec = glm::cross(v2, plane.normal());
        angle = angle2;
    }

    glm::mat4 rotmat = glm::rotate(glm::mat4(), angle, rotvec);

    v1 = (rotmat * glm::vec4(v1, 1)).xyz();
    v2 = (rotmat * glm::vec4(v2, 1)).xyz();

    glm::vec3 diru = v1 - plane.point();
    glm::vec3 dirv = v2 - plane.point();

    glm::vec3 startorig = start - plane.point();
    glm::vec3 negdir = -dir;
    glm::vec3 v2crossnegdir = glm::cross(dirv, negdir);

    float det = glm::dot(diru, v2crossnegdir);
    float det1 = glm::dot(startorig, v2crossnegdir);
    float det2 = glm::dot(diru, glm::cross(startorig, negdir));
    float det3 = glm::dot(diru, glm::cross(dirv, startorig));
    float tmpu, tmpv, tmpdist;
    tmpdist = det3/det;

    tmpu = det1/det;
    tmpv = det2/det;

    if(hitpoint) {
        glm::vec3 tmp1 = diru * tmpu;
        glm::vec3 tmp2 = dirv * tmpv;
        *hitpoint = tmp1 + tmp2;
        *hitpoint = *hitpoint + plane.point();
    }
    return true;
}

bool Ray::intersectTriangle(const glm::vec3 &p1, 
                            const glm::vec3 &p2, 
                            const glm::vec3 &p3, 
                            glm::vec3 *uvdist, 
                            glm::vec3 *hitpoint) const    
{
    glm::vec3 diru = p2 - p1;
    glm::vec3 dirv = p3 - p1;

    glm::vec3 startorig = start - p1;

    float det = glm::dot(glm::cross(diru, dirv), -dir);
    float det1 = glm::dot(glm::cross(startorig, dirv), -dir);
    float det2 = glm::dot(glm::cross(diru, startorig), -dir);
    float det3 = glm::dot(glm::cross(diru, dirv), startorig);

    float tmpu = det1/det;
    float tmpv = det2/det;
    float tmpdist = det3/det;

    if(uvdist) {
        uvdist->x = tmpu;
        uvdist->y = tmpv;
        uvdist->z = tmpdist;
    }
    if(hitpoint) {
        *hitpoint = p1 + diru * tmpu + dirv * tmpv;
    }

    if((tmpu + tmpv) < 1 && tmpu > 0 && tmpv > 0)
        return true;

    return false;
}


bool Ray::intersectRectangle(const Rectangle &rect, glm::vec3 *hitpoint) const
{
    glm::vec3 _hitpoint, uvdist;

    auto bl = rect.bottomLeft();
    auto u = rect.uvector();
    auto v = rect.vvector();

    intersectTriangle(bl, bl + u, bl + v, &uvdist, &_hitpoint);

    auto hit = start + dir * uvdist.z;
    if (hitpoint)
        *hitpoint = _hitpoint;
    if(uvdist.x > 0 && uvdist.x < 1 && uvdist.y > 0 && uvdist.y < 1) {
        return true;
    }
    return false;
}

bool Ray::intersectBox(const Box &box, glm::vec3 *hitpoint) const 
{
    return false;
}

bool Ray::intersectSphere(const Sphere &sphere, glm::vec3 *hitpoint) const
{
    return false;
}

Ray Ray::primaryRay(glm::mat4 mvp, glm::vec3 camPos, glm::ivec2 pixel, glm::ivec2 viewportSize)
{
    glm::vec2 pos = (glm::vec2(pixel) + glm::vec2(0.5)) / glm::vec2(viewportSize);
    pos *= 2;
    pos -= glm::vec2(1);

    auto invmvp = glm::inverse(mvp);

    auto farPos = (invmvp * glm::vec4(pos, 1., 1.));
    farPos /= farPos.w;
    auto dir = glm::normalize(farPos.xyz() - camPos);

     return Ray(camPos, dir);
}
