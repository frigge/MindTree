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

#include "data/nodes/data_node.h"
#include "data/dnspace.h"
#include "data/frg.h"
#include "data/nodes/node_db.h"

#include "buildin_nodes.h"

using namespace MindTree;

//DNode* BuildIn::MaddNode()
//{
//    MathNode *node = new MathNode(ADD);
//    return node;
//}
//
//DNode* BuildIn::MSubNode()
//{
//    MathNode *node = new MathNode(SUBTRACT);
//    return node;
//}
//
//DNode* BuildIn::MmultNode()
//{
//    MathNode *node = new MathNode(MULTIPLY);
//    return node;
//}
//
//DNode* BuildIn::MdivNode()
//{
//    MathNode *node = new MathNode(DIVIDE);
//    return node;
//}
//
//DNode* BuildIn::MdotNode()
//{
//    MathNode *node = new MathNode(DOTPRODUCT);
//    return node;
//}
//
//DNode* BuildIn::MmodNode()
//{
//    MathNode *node = new MathNode(MODULO);
//    return node;
//}
//
DNode* BuildIn::CandNode()
{
    ConditionNode *node = new ConditionNode(AND);
    return node;
}

DNode* BuildIn::CorNode()
{
    ConditionNode *node = new ConditionNode(OR);
    return node;
}

DNode* BuildIn::CnotNode()
{
    ConditionNode *node = new ConditionNode(NOT);
    return node;
}

DNode* BuildIn::CgreaterNode()
{
    ConditionNode *node = new ConditionNode(GREATERTHAN);
    return node;
}

DNode* BuildIn::CsmallerNode()
{
    ConditionNode *node = new ConditionNode(SMALLERTHAN);
    return node;
}

DNode* BuildIn::CeqNode()
{
    ConditionNode *node = new ConditionNode(EQUAL);
    return node;
}

void BuildIn::registerNodes()    
{
    //FRG::lib->addNode("Read File", "File IO", new BuildInFactory(createNode<ReadFileNode>)); 
    //FRG::lib->addNode("Write File", "File IO", new BuildInFactory(createNode<WriteFileNode>)); 
    //FRG::lib->addNode("Read Image", "File IO", new BuildInFactory(createNode<LoadImageNode>)); 
    //FRG::lib->addNode("Write Image", "File IO", new BuildInFactory(createNode<SaveImageNode>)); 

    //FRG::lib->addNode("External Process", "System", new BuildInFactory(createNode<ProcessNode>)); 

    //FRG::lib->addGroup("RSL Inputs", "Shading");
    //FRG::lib->addNode("Surface Input", "RSL Inputs", new BuildInFactory(createNode<SurfaceInputNode>)); 
//    FRG::lib->addNode("Displacement Input", "RSL Inputs", new BuildInFactory(createNode<DisplacementInputNode>)); 
//    FRG::lib->addNode("Light Input", "RSL Inputs", new BuildInFactory(createNode<LightInputNode>)); 
//    FRG::lib->addNode("Volume Input", "RSL Inputs", new BuildInFactory(createNode<VolumeInputNode>)); 
//
//    FRG::lib->addGroup("RSL Outputs", "Shading");
//    FRG::lib->addNode("Surface Output", "RSL Outputs", new BuildInFactory(createNode<SurfaceOutputNode>)); 
//    FRG::lib->addNode("Displacement Output", "RSL Outputs", new BuildInFactory(createNode<DisplacementOutputNode>)); 
//    FRG::lib->addNode("Volume Output", "RSL Outputs", new BuildInFactory(createNode<VolumeOutputNode>)); 
//    FRG::lib->addNode("Light Output", "RSL Outputs", new BuildInFactory(createNode<LightOutputNode>)); 
//    FRG::lib->addNode("Preview", "RSL Outputs", new BuildInFactory(createNode<DShaderPreview>)); 
//
//    FRG::lib->addGroup("GLSL Inputs", "Shading");
//    FRG::lib->addNode("Fragment Input", "GLSL Inputs", new BuildInFactory(createNode<GLFragmentInputNode>)); 
//    FRG::lib->addNode("Vertex Input", "GLSL Inputs", new BuildInFactory(createNode<GLVertexInputNode>)); 
//    FRG::lib->addNode("Geometry Input", "GLSL Inputs", new BuildInFactory(createNode<GLGeometryInputNode>)); 
//
//    FRG::lib->addGroup("GLSL Outputs", "Shading");
//    FRG::lib->addNode("Fragment Output", "GLSL Outputs", new BuildInFactory(createNode<FragmentOutputNode>)); 
//    FRG::lib->addNode("Vertex Output", "GLSL Outputs", new BuildInFactory(createNode<VertexOutputNode>)); 
//    FRG::lib->addNode("Geometry Output", "GLSL Outputs", new BuildInFactory(createNode<GeometryOutputNode>)); 
//
    //FRG::lib->addNode("Add", "Math", new BuildInFactory(&MaddNode)); 
    //FRG::lib->addNode("Subtract", "Math", new BuildInFactory(&MSubNode)); 
    //FRG::lib->addNode("Multiply", "Math", new BuildInFactory(&MmultNode)); 
    //FRG::lib->addNode("Divide", "Math", new BuildInFactory(&MdivNode)); 
    //FRG::lib->addNode("Dotproduct", "Math", new BuildInFactory(&MdotNode)); 
    //FRG::lib->addNode("Modulo", "Math", new BuildInFactory(&MmodNode)); 

    //FRG::lib->addNode("And", "Boolean", new BuildInFactory(&CandNode)); 
    //FRG::lib->addNode("Or", "Boolean", new BuildInFactory(&CorNode)); 
    //FRG::lib->addNode("Not", "Boolean", new BuildInFactory(&CnotNode)); 
    //FRG::lib->addNode("Greater Than", "Boolean", new BuildInFactory(&CgreaterNode)); 
    //FRG::lib->addNode("Smaller Than", "Boolean", new BuildInFactory(&CsmallerNode)); 
    //FRG::lib->addNode("Equal", "Boolean", new BuildInFactory(&CeqNode)); 

    //FRG::lib->addNode("Color", "Value", new BuildInFactory(createNode<ColorValueNode>)); 
    //FRG::lib->addNode("String", "Value", new BuildInFactory(createNode<StringValueNode>)); 
    //FRG::lib->addNode("Float", "Value", new BuildInFactory(createNode<FloatValueNode>)); 
    //FRG::lib->addNode("Integer", "Value", new BuildInFactory(createNode<IntValueNode>)); 
    //FRG::lib->addNode("Vector", "Value", new BuildInFactory(createNode<VectorValueNode>)); 
    //FRG::lib->addNode("Boolean", "Value", new BuildInFactory(createNode<BoolValueNode>)); 

    //FRG::lib->addNode("FloatToVector", "Conversion", new BuildInFactory(createNode<FloatToVectorNode>));

    //FRG::lib->addNode("If", "Boolean", new BuildInFactory(createNode<ConditionContainerNode>)); 
    //FRG::lib->addNode("For", "Loop", new BuildInFactory(createNode<ForNode>)); 
    //FRG::lib->addNode("While", "Loop", new BuildInFactory(createNode<WhileNode>)); 
    //FRG::lib->addNode("Foreach", "Loop", new BuildInFactory(createNode<ForeachNode>)); 
    
//    FRG::lib->addGroup("Shading Loop", "Shading");
//    FRG::lib->addNode("Illuminate", "Shading Loop", new BuildInFactory(createNode<IlluminateNode>)); 
//    FRG::lib->addNode("Illuminance", "Shading Loop", new BuildInFactory(createNode<IlluminanceNode>)); 
//    FRG::lib->addNode("Solar", "Shading Loop", new BuildInFactory(createNode<SolarNode>)); 
//    FRG::lib->addNode("Gather", "Shading Loop", new BuildInFactory(createNode<GatherNode>)); 
//
    //FRG::lib->addNode("Get Array", "Array", new BuildInFactory(createNode<GetArrayNode>));
    //FRG::lib->addNode("Set Array", "Array", new BuildInFactory(createNode<SetArrayNode>));
    //FRG::lib->addNode("Compose Array", "Array", new BuildInFactory(createNode<ComposeArrayNode>));

    //FRG::lib->addNode("Varname", "Misc", new BuildInFactory(createNode<VarNameNode>));
    
}
