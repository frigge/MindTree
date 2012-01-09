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

#include "buildin_nodes.h"

#include "data_node.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/graphics/viewport.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/nodes/data_node.h"
#include "source/data/base/project.h"
#include "source/data/scene/object.h"
#include "source/graphics/shaderpreview.h"
#include "source/data/code_generator/inputs.h"
#include "source/data/code_generator/outputs.h"

DNode* BuildIn::MaddNode()
{
    MathNode *node = new MathNode(ADD);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::MSubNode()
{
    MathNode *node = new MathNode(SUBTRACT);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::MmultNode()
{
    MathNode *node = new MathNode(MULTIPLY);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::MdivNode()
{
    MathNode *node = new MathNode(DIVIDE);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::MdotNode()
{
    MathNode *node = new MathNode(DOTPRODUCT);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::MmodNode()
{
    MathNode *node = new MathNode(MODULO);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CandNode()
{
    ConditionNode *node = new ConditionNode(AND);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CorNode()
{
    ConditionNode *node = new ConditionNode(OR);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CnotNode()
{
    ConditionNode *node = new ConditionNode(NOT);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CgreaterNode()
{
    ConditionNode *node = new ConditionNode(GREATERTHAN);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CsmallerNode()
{
    ConditionNode *node = new ConditionNode(SMALLERTHAN);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CeqNode()
{
    ConditionNode *node = new ConditionNode(EQUAL);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

void BuildIn::registerNodes()    
{
    FRG::lib->addGroup("RSL Inputs", "Shading");
    FRG::lib->addNode("Surface Input", "RSL Inputs", createNode<SurfaceInputNode>); 
    FRG::lib->addNode("Displacement Input", "RSL Inputs", createNode<DisplacementInputNode>); 
    FRG::lib->addNode("Light Input", "RSL Inputs", createNode<LightInputNode>); 
    FRG::lib->addNode("Volume Input", "RSL Inputs", createNode<VolumeInputNode>); 

    FRG::lib->addGroup("RSL Outputs", "Shading");
    FRG::lib->addNode("Surface Output", "RSL Outputs", createNode<SurfaceOutputNode>); 
    FRG::lib->addNode("Displacement Output", "RSL Outputs", createNode<DisplacementOutputNode>); 
    FRG::lib->addNode("Volume Output", "RSL Outputs", createNode<VolumeOutputNode>); 
    FRG::lib->addNode("Light Output", "RSL Outputs", createNode<LightOutputNode>); 
    FRG::lib->addNode("Preview", "RSL Outputs", createNode<DShaderPreview>); 

    FRG::lib->addGroup("GLSL Inputs", "Shading");
    FRG::lib->addNode("Fragment Input", "GLSL Inputs", createNode<GLFragmentInputNode>); 
    FRG::lib->addNode("Vertex Input", "GLSL Inputs", createNode<GLVertexInputNode>); 
    FRG::lib->addNode("Geometry Input", "GLSL Inputs", createNode<GLGeometryInputNode>); 

    FRG::lib->addGroup("GLSL Outputs", "Shading");
    FRG::lib->addNode("Fragment Output", "GLSL Outputs", createNode<FragmentOutputNode>); 
    FRG::lib->addNode("Vertex Output", "GLSL Outputs", createNode<VertexOutputNode>); 
    FRG::lib->addNode("Geometry Output", "GLSL Outputs", createNode<GeometryOutputNode>); 

    FRG::lib->addNode("Add", "Math", &MaddNode); 
    FRG::lib->addNode("Subtract", "Math", &MSubNode); 
    FRG::lib->addNode("Multiply", "Math", &MmultNode); 
    FRG::lib->addNode("Divide", "Math", &MdivNode); 
    FRG::lib->addNode("Dotproduct", "Math", &MdotNode); 
    FRG::lib->addNode("Modulo", "Math", &MmodNode); 

    FRG::lib->addNode("And", "Boolean", &CandNode); 
    FRG::lib->addNode("Or", "Boolean", &CorNode); 
    FRG::lib->addNode("Not", "Boolean", &CnotNode); 
    FRG::lib->addNode("Greater Than", "Boolean", &CgreaterNode); 
    FRG::lib->addNode("Smaller Than", "Boolean", &CsmallerNode); 
    FRG::lib->addNode("Equal", "Boolean", &CeqNode); 

    FRG::lib->addNode("Color", "Value", createNode<ColorValueNode>); 
    FRG::lib->addNode("String", "Value", createNode<StringValueNode>); 
    FRG::lib->addNode("Float", "Value", createNode<FloatValueNode>); 
    FRG::lib->addNode("Integer", "Value", createNode<IntValueNode>); 
    FRG::lib->addNode("Vector", "Value", createNode<VectorValueNode>); 
    FRG::lib->addNode("Boolean", "Value", createNode<BoolValueNode>); 

    FRG::lib->addNode("FloatToVector", "Conversion", createNode<FloatToVectorNode>);

    FRG::lib->addNode("If", "Boolean", createNode<ConditionContainerNode>); 
    FRG::lib->addNode("For", "Loop", createNode<ForNode>); 
    FRG::lib->addNode("While", "Loop", createNode<WhileNode>); 
    FRG::lib->addNode("Foreach", "Loop", createNode<ForeachNode>); 
    
    FRG::lib->addGroup("Shading Loop", "Shading");
    FRG::lib->addNode("Illuminate", "Shading Loop", createNode<IlluminateNode>); 
    FRG::lib->addNode("Illuminance", "Shading Loop", createNode<IlluminanceNode>); 
    FRG::lib->addNode("Solar", "Shading Loop", createNode<SolarNode>); 
    FRG::lib->addNode("Gather", "Shading Loop", createNode<GatherNode>); 

    FRG::lib->addNode("Get Array", "Array", createNode<GetArrayNode>);
    FRG::lib->addNode("Set Array", "Array", createNode<SetArrayNode>);
    FRG::lib->addNode("Compose Array", "Array", createNode<ComposeArrayNode>);

    FRG::lib->addNode("Varname", "Misc", createNode<VarNameNode>);
    
    FRG::lib->addNode("Viewport", "Data", createNode<ViewportNode>); 
    FRG::lib->addNode("Compose Object", "Data", createNode<ObjectNode>); 
    FRG::lib->addNode("Compose Polygon", "Data", createNode<PolygonNode>); 
}
