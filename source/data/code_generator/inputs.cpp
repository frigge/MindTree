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
    setNodeType(SURFACEINPUT);
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
    setNodeType(DISPLACEMENTINPUT);
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
    setNodeType(VOLUMEINPUT);
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
    setNodeType(LIGHTINPUT);
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
