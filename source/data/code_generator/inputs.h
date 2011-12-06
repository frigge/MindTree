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

#ifndef INPUTS_4BXR1560

#define INPUTS_4BXR1560

#include "data_node.h"

class InputNode : public DNode
{
public:
    InputNode(QString name);
    InputNode(const InputNode* node);
};

//RSL Input Nodes >>
class SurfaceInputNode : public InputNode
{
public:
    SurfaceInputNode(bool raw=false);
    SurfaceInputNode(const SurfaceInputNode *node);
};

class DisplacementInputNode : public InputNode
{
public:
    DisplacementInputNode(bool raw=false);
    DisplacementInputNode(const DisplacementInputNode *node);
};

class LightInputNode : public InputNode
{
public:
    LightInputNode(bool raw=false);
    LightInputNode(const LightInputNode *node);
};

class VolumeInputNode : public InputNode
{
public:
    VolumeInputNode(bool raw=false);
    VolumeInputNode(const VolumeInputNode *node);
};

class IlluminanceInputNode : public InputNode
{
public:
    IlluminanceInputNode(bool raw=false);
    IlluminanceInputNode(const IlluminanceInputNode *node);
};

class IlluminateInputNode : public InputNode
{
public:
    IlluminateInputNode(bool raw=false);
    IlluminateInputNode(const IlluminateInputNode *node);
};
//RSL Input Nodes <<

//GLSL Input Nodes >>
class GLFragmentInputNode : public InputNode
{
public:
    GLFragmentInputNode(bool raw=false);
    GLFragmentInputNode(const GLFragmentInputNode *node);
};

class GLVertexInputNode : public InputNode
{
public:
    GLVertexInputNode(bool raw=false);
    GLVertexInputNode(const GLVertexInputNode *node);
};

class GLGeometryInputNode : public InputNode
{
public:
    GLGeometryInputNode(bool raw=false);
    GLGeometryInputNode(const GLGeometryInputNode *node);
};
//GLSL Input Nodes <<
#endif /* end of include guard: INPUTS_4BXR1560 */

