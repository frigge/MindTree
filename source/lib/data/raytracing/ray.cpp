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


#include "data/debuglog.h"
#define GLM_FORCE_SWIZZLE
#include "iostream"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "ray.h"

Ray::Ray(glm::vec3 start, glm::vec3 d)
    : start(start), dir(d)
{
}

Ray::~Ray()
{
}

bool Ray::intersect(const Plane &plane, float *depth) const
{
	glm::vec3 p = plane.point();
	glm::vec3 n = plane.normal();
	float dirn = glm::dot(dir, n);

	//avoid rays that are almost perpendicular (precision errors)
	if(abs(dirn) < 0.00001f)
		return false;

	float t = (glm::dot(p, n) - glm::dot(start, n)) / dirn;

	*depth = t;

    return true;
}

bool Ray::intersect(const Ray &r, float *depth) const
{
	//find plane perpendicular to both rays
	glm::vec3 n = glm::cross(r.dir, dir);
	Plane p{r.start, n};

	float closest{std::numeric_limits<float>::max()};
	Ray(start, n).intersect(p, &closest);

	//project both rays into plane and find 2d intersection

	glm::vec3 x = glm::normalize(dir);
	glm::vec3 y = glm::normalize(glm::cross(x, n));

	glm::mat3 m(x, y, n);
	glm::mat3 invm = glm::inverse(m);

	//homogeneous representation of 2D lines in plane
	glm::vec3 line = glm::cross(glm::vec3((invm * start).xy(), 1),
	                            glm::vec3((invm * (start + dir)).xy(), 1));
	glm::vec3 rline = glm::cross(glm::vec3((invm * r.start).xy(), 1),
	                             glm::vec3((invm * (r.start + r.dir)).xy(), 1));

	glm::vec3 intersection = m * glm::cross(line, rline);
	*depth = glm::distance(start, intersection);

	if (abs(closest) > .2)
		return false;

	return true;
}

bool Ray::intersect(const glm::vec3 &p1,
                    const glm::vec3 &p2,
                    const glm::vec3 &p3,
                    float *depth,
                    glm::vec2 *uv) const
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

	if((tmpu + tmpv) < 1 && tmpu > 0 && tmpv > 0) {
		*depth = tmpdist;
		if(uv) {
			uv->x = tmpu;
			uv->y = tmpv;
		}
		return true;
	}
	
    return false;
}


bool Ray::intersect(const Rectangle &rect, float *depth) const
{
	glm::vec2 uv;

	auto bl = rect.bottomLeft();
    auto u = rect.uvector();
    auto v = rect.vvector();

    auto n = rect.normal();
    if (glm::dot(dir, n) < 0.f) n *= -1.f;

    if (!intersect(Plane(bl, n), depth))
		return false;

    glm::vec3 p =  start + *depth * dir;

    auto pbl = p - bl;
	uv.x = glm::dot(pbl, u) / glm::dot(u, u);
	uv.y = glm::dot(pbl, v) / glm::dot(v, v);

	if(uv.x > 0 && uv.x < 1 && uv.y > 0 && uv.y < 1) {
		return true;
    }
	return false;
}

bool Ray::intersect(const Box &box, float *depth) const
{
    return false;
}

bool Ray::intersect(const Sphere &sphere, float *depth) const
{
    return false;
}

Ray Ray::primaryRay(glm::ivec2 pixel, glm::ivec2 size, float fov)
{
    glm::vec2 pos = (glm::vec2(pixel) + glm::vec2(0.5)) / glm::vec2(size);
    pos *= 2;
    pos -= glm::vec2(1);

    float aspect = (float)size.x / size.y;

	glm::vec4 dir;
	dir.x = tan(fov * 0.5f * aspect) * pos.x;
	dir.y = tan(fov * 0.5f) * pos.y;
	dir.z = -1;

	return Ray(glm::vec3(0), glm::normalize(dir.xyz()));
}
