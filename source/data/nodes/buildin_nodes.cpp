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
#include "source/data/base/frg.h"
#include "source/data/base/project.h"
#include "source/data/scene/object.h"
#include "source/graphics/shaderpreview.h"

DNode* BuildIn::getArray()
{
    GetArrayNode *node = new GetArrayNode();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::setArray()
{
    SetArrayNode *node = new SetArrayNode();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::composeArray()
{
    ComposeArrayNode *node = new ComposeArrayNode();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VarName()
{
    VarNameNode *node = new VarNameNode();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::surfaceInput()
{
    InputNode *node = new InputNode();
    DNode::setsurfaceInput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::displacementInput()
{
    InputNode *node = new InputNode();
    DNode::setdisplacementInput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::volumeInput()
{
    InputNode *node = new InputNode();
    DNode::setvolumeInput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::lightInput()
{
    InputNode *node = new InputNode();
    DNode::setlightInput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::surfaceOutput()
{
    OutputNode *node = new OutputNode();
    DNode::setsurfaceOutput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
};

DNode* BuildIn::displacementOutput()
{
    OutputNode *node = new OutputNode();
    DNode::setdisplacementOutput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::volumeOutput()
{
    OutputNode *node = new OutputNode();
    DNode::setvolumeOutput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::lightOutput()
{
    OutputNode *node = new OutputNode();
    DNode::setlightOutput(node);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::preview()    
{
    DShaderPreview *node = new DShaderPreview();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

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

DNode* BuildIn::VColNode()
{
    ColorValueNode *node = new ColorValueNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VStrNode()
{
    StringValueNode *node = new StringValueNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VFlNode()
{
    FloatValueNode *node = new FloatValueNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VIntNode()
{
    IntValueNode *node = new IntValueNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VVecNode()
{
    VectorValueNode *node = new VectorValueNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VFloatToVector()
{
    FloatToVectorNode *node = new FloatToVectorNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::VBoolNode()
{
    BoolValueNode *node = new BoolValueNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::ContIfNode()
{
    ConditionContainerNode *node = new ConditionContainerNode;
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::ContForNode()
{
    ForNode *node = new ForNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::ContWhileNode()
{
    WhileNode *node = new WhileNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CLilluminate()
{
    IlluminateNode *node = new IlluminateNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CLgather()
{
    GatherNode *node = new GatherNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CLilluminance()
{
    IlluminanceNode *node = new IlluminanceNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::CLsolar()
{
    SolarNode *node = new SolarNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::Viewport()
{
    ViewportNode *node = new ViewportNode(false);
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::ComposePolygon()
{
    PolygonNode *node = new PolygonNode();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

DNode* BuildIn::ComposeObject()
{
    ObjectNode *node = new ObjectNode();
    FRG::SpaceDataInFocus->addNode(node);
    return node;
}

void BuildIn::registerNodes()    
{
    FRG::lib->addGroup("Inputs", "Shading");
    FRG::lib->addNode("Surface Input", "Inputs", &surfaceInput); 
    FRG::lib->addNode("Displacement Input", "Inputs", &displacementInput); 
    FRG::lib->addNode("Light Input", "Inputs", &lightInput); 
    FRG::lib->addNode("Volume Input", "Inputs", &volumeInput); 

    FRG::lib->addGroup("Outputs", "Shading");
    FRG::lib->addNode("Surface Output", "Outputs", &surfaceOutput); 
    FRG::lib->addNode("Displacement Output", "Outputs", &displacementOutput); 
    FRG::lib->addNode("Volume Output", "Outputs", &volumeOutput); 
    FRG::lib->addNode("Light Output", "Outputs", &lightOutput); 
    FRG::lib->addNode("Preview", "Outputs", &preview); 

    FRG::lib->addNode("Add", "Math", &MaddNode); 
    FRG::lib->addNode("Subtract", "Math", &MSubNode); 
    FRG::lib->addNode("Multiply", "Math", &MmultNode); 
    FRG::lib->addNode("Divide", "Math", &MdivNode); 
    FRG::lib->addNode("Dotproduct", "Math", &MdotNode); 

    FRG::lib->addNode("And", "Boolean", &CandNode); 
    FRG::lib->addNode("Or", "Boolean", &CorNode); 
    FRG::lib->addNode("Not", "Boolean", &CnotNode); 
    FRG::lib->addNode("Greater Than", "Boolean", &CgreaterNode); 
    FRG::lib->addNode("Smaller Than", "Boolean", &CsmallerNode); 
    FRG::lib->addNode("Equal", "Boolean", &CeqNode); 

    FRG::lib->addNode("Color", "Value", &VColNode); 
    FRG::lib->addNode("String", "Value", &VStrNode); 
    FRG::lib->addNode("Float", "Value", &VFlNode); 
    FRG::lib->addNode("Integer", "Value", &VIntNode); 
    FRG::lib->addNode("Vector", "Value", &VVecNode); 
    FRG::lib->addNode("Boolean", "Value", &VBoolNode); 

    FRG::lib->addNode("FloatToVector", "Conversion", &VFloatToVector);

    FRG::lib->addNode("If", "Boolean", &ContIfNode); 
    FRG::lib->addNode("For", "Loop", &ContForNode); 
    FRG::lib->addNode("While", "Loop", &ContWhileNode); 
    
    FRG::lib->addGroup("Shading Loop", "Shading");
    FRG::lib->addNode("Illuminate", "Shading Loop", &CLilluminate); 
    FRG::lib->addNode("Illuminance", "Shading Loop", &CLilluminance); 
    FRG::lib->addNode("Solar", "Shading Loop", &CLsolar); 
    FRG::lib->addNode("Gather", "Shading Loop", &CLgather); 

    FRG::lib->addNode("Get Array", "Array", &getArray);
    FRG::lib->addNode("Set Array", "Array", &setArray);
    FRG::lib->addNode("Compose Array", "Array", &composeArray);

    FRG::lib->addNode("Varname", "Misc", &VarName);
    
    FRG::lib->addNode("Viewport", "Data", &Viewport); 
    FRG::lib->addNode("Compose Object", "Data", &ComposeObject); 
    FRG::lib->addNode("Compose Polygon", "Data", &ComposePolygon); 
}
