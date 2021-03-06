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

#include "wrapper.h"
#include "pyexposable.h"

using namespace MindTree;

PyExposable::PyExposable()
{
}

PyExposable::~PyExposable()
{
    for(PyWrapper *wrapper : wrapper_objects){
        wrapper->elementDestroyed();
    }
}

void PyExposable::regWrapper(PyWrapper *wrapper)    
{
    wrapper_objects.push_back(wrapper);
}

void PyExposable::rmWrapper(PyWrapper *wrapper)    
{
    auto begin = wrapper_objects.begin();
    auto end = wrapper_objects.end();

    auto it = std::find(begin, end, wrapper);

    if(it != end) {
        wrapper_objects.erase(it);
    }
}
