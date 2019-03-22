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

#ifndef RAY

#define RAY

#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp"
#include "graphics/shapes.h"
#include "memory"

class Camera;

class Ray
{
public:
    glm::vec3 start, dir;

    Ray(glm::vec3 start, glm::vec3 dir);
    virtual ~Ray();

    bool intersect(const Ray &r, float *depth) const;
    bool intersect(const Plane &plane, float *depth) const;
    bool intersect(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, float *depth, glm::vec2 *uv=nullptr) const;
    bool intersect(const Rectangle &rect, float *depth) const;
    bool intersect(const Box &box, float *depth) const;
    bool intersect(const Sphere &sphere, float *depth) const;

	static Ray primaryRay(glm::ivec2 pixel, glm::ivec2 size, float fov);
};

#endif /* end of include guard: RAY*/
