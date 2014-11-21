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

#include "inputs.h"

using namespace MindTree;
InputNode::InputNode(QString name)
    : DNode(name)
{
}

InputNode::InputNode(const InputNode* node)
    : DNode(node)
{
}

SurfaceInputNode::SurfaceInputNode(bool raw)
    : InputNode("Surace Input")
{
    setType(SURFACEINPUT);
    if(!raw){
        new DoutSocket("P", POINT, this);
        new DoutSocket("N", NORMAL, this);
        new DoutSocket("Cs", COLOR, this);
        new DoutSocket("Os", COLOR, this);
        new DoutSocket("u", FLOAT, this);
        new DoutSocket("v", FLOAT, this);
        new DoutSocket("du", FLOAT, this);
        new DoutSocket("dv", FLOAT, this);
        new DoutSocket("s", FLOAT, this);
        new DoutSocket("t", FLOAT, this);
        new DoutSocket("I", VECTOR, this);
    }
}

SurfaceInputNode::SurfaceInputNode(const SurfaceInputNode *node)
    : InputNode(node)
{
}

DisplacementInputNode::DisplacementInputNode(bool raw)
    : InputNode("Displacement Input")
{
    setType(DISPLACEMENTINPUT);
    if (!raw) {
        new DoutSocket("P", POINT, this);
        new DoutSocket("N", NORMAL, this);
        new DoutSocket("u", FLOAT, this);
        new DoutSocket("v", FLOAT, this);
        new DoutSocket("du", FLOAT, this);
        new DoutSocket("dv", FLOAT, this);
        new DoutSocket("s", FLOAT, this);
        new DoutSocket("t", FLOAT, this);
    }
}

DisplacementInputNode::DisplacementInputNode(const DisplacementInputNode *node)
    : InputNode(node)
{
}

VolumeInputNode::VolumeInputNode(bool raw)
    : InputNode("Volume Input")
{
    setType(VOLUMEINPUT);
    if(!raw){
        new DoutSocket("P", POINT, this);
        new DoutSocket("I", VECTOR, this);
        new DoutSocket("Ci", COLOR, this);
        new DoutSocket("Oi", COLOR, this);
        new DoutSocket("Cs", COLOR, this);
        new DoutSocket("Os", COLOR, this);
        new DoutSocket("L", VECTOR, this);
        new DoutSocket("Cl", COLOR, this);
    }
}

VolumeInputNode::VolumeInputNode(const VolumeInputNode *node)
    : InputNode(node)
{
}

LightInputNode::LightInputNode(bool raw)
    : InputNode("Light Input")
{
    setType(LIGHTINPUT);
    if(!raw){
        new DoutSocket("P", POINT, this);
        new DoutSocket("Ps", POINT, this);
        new DoutSocket("L", VECTOR, this);
    }
}

LightInputNode::LightInputNode(const LightInputNode *node)
    : InputNode(node)
{
}

IlluminanceInputNode::IlluminanceInputNode(bool raw)
    : InputNode("Illuminance Input")
{
}

IlluminanceInputNode::IlluminanceInputNode(const IlluminanceInputNode *node)
    : InputNode(node)
{
}

IlluminateInputNode::IlluminateInputNode(bool raw)
    : InputNode("Illuminate Input")
{
}

IlluminateInputNode::IlluminateInputNode(const IlluminateInputNode *node)
    : InputNode(node)
{
}

GLFragmentInputNode::GLFragmentInputNode(bool raw)
    : InputNode("Fragment Shader Input")
{
    setType(GLFRAGMENTINPUT);
    if(!raw){
        new DoutSocket("gl_Color", COLOR, this);
        new DoutSocket("gl_SecondaryColor", COLOR, this);
        new DoutSocket("gl_TexCoord[]", COLOR, this);
        new DoutSocket("gl__FogFragCoord", FLOAT, this);
        new DoutSocket("gl_FragCoord", POINT, this);
        new DoutSocket("gl_FrontFacing", CONDITION, this);
    }
}

GLFragmentInputNode::GLFragmentInputNode(const GLFragmentInputNode *node)
    : InputNode(node)
{
}

GLVertexInputNode::GLVertexInputNode(bool raw)
    : InputNode("Vertex Shader Input")
{
    setType(GLVERTEXINPUT);
    if(!raw){
        new DoutSocket("gl_Vertex", VECTOR, this);
        new DoutSocket("gl_Normal", NORMAL, this);
        new DoutSocket("gl_Color", COLOR, this);
        new DoutSocket("gl_SecondaryColor", COLOR, this);
        new DoutSocket("gl_MultiTexCoord0", POINT, this);
        new DoutSocket("gl_MultiTexCoord1", POINT, this);
        new DoutSocket("gl_MultiTexCoord2", POINT, this);
        new DoutSocket("gl_MultiTexCoord3", POINT, this);
        new DoutSocket("gl_MultiTexCoord4", POINT, this);
        new DoutSocket("gl_MultiTexCoord5", POINT, this);
        new DoutSocket("gl_MultiTexCoord6", POINT, this);
        new DoutSocket("gl_MultiTexCoord7", POINT, this);
        new DoutSocket("gl_FogCoord", FLOAT, this);
    }
}

GLVertexInputNode::GLVertexInputNode(const GLVertexInputNode *node)
    : InputNode(node)
{
}

GLGeometryInputNode::GLGeometryInputNode(bool raw)
    : InputNode("Geometry Shader Input")
{
    setType(GLGEOMETRYINPUT);
    if(!raw){
        new DoutSocket("gl_VerticesIn", INTEGER, this);
        new DoutSocket("gl_FrontColorIn", COLOR, this);
        new DoutSocket("gl_BackColorIn", COLOR, this);
        new DoutSocket("gl_FrontSecondaryColorIn", COLOR, this);
        new DoutSocket("gl_BackSecondaryColorIn", COLOR, this);
        new DoutSocket("gl_TexCoordIn", POINT, this);
        new DoutSocket("gl_FogFragCoordIn", POINT, this);
        new DoutSocket("gl_PositionIn", POINT, this);
        new DoutSocket("gl_PointSizeIn", POINT, this);
        new DoutSocket("gl_ClipVertexIn", POINT, this);
    }
}

GLGeometryInputNode::GLGeometryInputNode(const GLGeometryInputNode *node)
    : InputNode(node)
{
}

