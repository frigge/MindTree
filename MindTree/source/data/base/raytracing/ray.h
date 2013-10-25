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

#include "data/properties.h"
#include "data/datatypes.h"

class Ray
{
public:
    Ray(Vector start, Vector dir);
    virtual ~Ray();
    bool intersectPlane(Vector orig, Vector v1, Vector v2, double *u=0, double *v=0, double *distance=0, Vector *hitpoint=0);

private:
    Vector start, dir;
};

#endif /* end of include guard: RAY*/
