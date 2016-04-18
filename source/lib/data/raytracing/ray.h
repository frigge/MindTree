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

#include "glm/glm.hpp"
#include "graphics/shapes.h"
#include "memory"

class Camera;

class Ray
{
public:
    Ray(glm::vec3 start, glm::vec3 dir);
    virtual ~Ray();

    bool intersect(const Plane &plane, glm::vec3 *hitpoint=nullptr) const;
    bool intersect(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, glm::vec3 *uvdist=nullptr, glm::vec3 *hitpoint=nullptr) const;
    bool intersect(const Rectangle &rect, glm::vec3 *hitpoint=nullptr) const;
    bool intersect(const Box &box, glm::vec3 *hitpoint=nullptr) const;
    bool intersect(const Sphere &sphere, glm::vec3 *hitpoint=nullptr) const;

    static Ray primaryRay(glm::mat4 mvp, glm::vec3 camPos, glm::ivec2 pixel, glm::ivec2 viewportSize);

private:
    glm::vec3 start, dir;
};

#endif /* end of include guard: RAY*/
