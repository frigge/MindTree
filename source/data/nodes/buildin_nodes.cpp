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

void* BuildIn::getArray(void *space)    
{
    GetArrayNode *node = new GetArrayNode();
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::setArray(void *space)    
{
    SetArrayNode *node = new SetArrayNode();
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::composeArray(void *space)    
{
    ComposeArrayNode *node = new ComposeArrayNode();
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VarName(void *space)    
{
    VarNameNode *node = new VarNameNode();
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::surfaceInput(void *space)
{
    InputNode *node = new InputNode();
    DNode::setsurfaceInput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::displacementInput(void *space)
{
    InputNode *node = new InputNode();
    DNode::setdisplacementInput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::volumeInput(void *space)
{
    InputNode *node = new InputNode();
    DNode::setvolumeInput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::lightInput(void *space)
{
    InputNode *node = new InputNode();
    DNode::setlightInput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::surfaceOutput(void *space)
{
    OutputNode *node = new OutputNode();
    DNode::setsurfaceOutput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
};

void* BuildIn::displacementOutput(void *space)
{
    OutputNode *node = new OutputNode();
    DNode::setdisplacementOutput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::volumeOutput(void *space)
{
    OutputNode *node = new OutputNode();
    DNode::setvolumeOutput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::lightOutput(void *space)
{
    OutputNode *node = new OutputNode();
    DNode::setlightOutput(node);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::MaddNode(void *space)
{
    MathNode *node = new MathNode(ADD);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::MSubNode(void *space)
{
    MathNode *node = new MathNode(SUBTRACT);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::MmultNode(void *space)
{
    MathNode *node = new MathNode(MULTIPLY);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::MdivNode(void *space)
{
    MathNode *node = new MathNode(DIVIDE);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::MdotNode(void *space)
{
    MathNode *node = new MathNode(DOTPRODUCT);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CandNode(void *space)
{
    ConditionNode *node = new ConditionNode(AND);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CorNode(void *space)
{
    ConditionNode *node = new ConditionNode(OR);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CnotNode(void *space)
{
    ConditionNode *node = new ConditionNode(NOT);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CgreaterNode(void *space)
{
    ConditionNode *node = new ConditionNode(GREATERTHAN);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CsmallerNode(void *space)
{
    ConditionNode *node = new ConditionNode(SMALLERTHAN);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CeqNode(void *space)
{
    ConditionNode *node = new ConditionNode(EQUAL);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VColNode(void *space)
{
    ColorValueNode *node = new ColorValueNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VStrNode(void *space)
{
    StringValueNode *node = new StringValueNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VFlNode(void *space)
{
    FloatValueNode *node = new FloatValueNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VIntNode(void *space)
{
    IntValueNode *node = new IntValueNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VVecNode(void *space)
{
    VectorValueNode *node = new VectorValueNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::VBoolNode(void *space)
{
    BoolValueNode *node = new BoolValueNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::ContIfNode(void *space)
{
    ConditionContainerNode *node = new ConditionContainerNode;
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::ContForNode(void *space)
{
    ForNode *node = new ForNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::ContWhileNode(void *space)
{
    WhileNode *node = new WhileNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CLilluminate(void *space)
{
    IlluminateNode *node = new IlluminateNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CLgather(void *space)
{
    GatherNode *node = new GatherNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CLilluminance(void *space)
{
    IlluminanceNode *node = new IlluminanceNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::CLsolar(void *space)
{
    SolarNode *node = new SolarNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::Viewport(void *space)
{
    ViewportNode *node = new ViewportNode(false);
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::ComposePolygon(void *space)    
{
    PolygonNode *node = new PolygonNode();
    ((DNSpace*)space)->addNode(node);
    return node;
}

void* BuildIn::ComposeObject(void *space)    
{
    ObjectNode *node = new ObjectNode();
    ((DNSpace*)space)->addNode(node);
    return node;
}

void BuildIn::registerNodes()    
{
    FRG::lib->addGroup("Inputs", "Shading");
    FRG::lib->addNode("Surface Input", "Inputs", surfaceInput); 
    FRG::lib->addNode("Displacement Input", "Inputs", displacementInput); 
    FRG::lib->addNode("Light Input", "Inputs", lightInput); 
    FRG::lib->addNode("Volume Input", "Inputs", volumeInput); 

    FRG::lib->addGroup("Outputs", "Shading");
    FRG::lib->addNode("Surface Output", "Outputs", surfaceOutput); 
    FRG::lib->addNode("Displacement Output", "Outputs", displacementOutput); 
    FRG::lib->addNode("Volume Output", "Outputs", volumeOutput); 
    FRG::lib->addNode("Light Output", "Outputs", lightOutput); 

    FRG::lib->addNode("Add", "Math", MaddNode); 
    FRG::lib->addNode("Subtract", "Math", MSubNode); 
    FRG::lib->addNode("Multiply", "Math", MmultNode); 
    FRG::lib->addNode("Divide", "Math", MdivNode); 
    FRG::lib->addNode("Dotproduct", "Math", MdotNode); 

    FRG::lib->addNode("And", "Boolean", CandNode); 
    FRG::lib->addNode("Or", "Boolean", CorNode); 
    FRG::lib->addNode("Not", "Boolean", CnotNode); 
    FRG::lib->addNode("Greater Than", "Boolean", CgreaterNode); 
    FRG::lib->addNode("Smaller Than", "Boolean", CsmallerNode); 
    FRG::lib->addNode("Equal", "Boolean", CeqNode); 

    FRG::lib->addNode("Color", "Value", VColNode); 
    FRG::lib->addNode("String", "Value", VStrNode); 
    FRG::lib->addNode("Float", "Value", VFlNode); 
    FRG::lib->addNode("Integer", "Value", VIntNode); 
    FRG::lib->addNode("Vector", "Value", VVecNode); 
    FRG::lib->addNode("Boolean", "Value", VBoolNode); 

    FRG::lib->addNode("If", "Boolean", ContIfNode); 
    FRG::lib->addNode("For", "Loop", ContForNode); 
    FRG::lib->addNode("While", "Loop", ContWhileNode); 
    
    FRG::lib->addGroup("Shading Loop", "Shading");
    FRG::lib->addNode("Illuminate", "Shading Loop", CLilluminate); 
    FRG::lib->addNode("Illuminance", "Shading Loop", CLilluminance); 
    FRG::lib->addNode("Solar", "Shading Loop", CLsolar); 
    FRG::lib->addNode("Gather", "Shading Loop", CLgather); 

    FRG::lib->addNode("Get Array", "Array", getArray);
    FRG::lib->addNode("Set Array", "Array", setArray);
    FRG::lib->addNode("Compose Array", "Array", composeArray);

    FRG::lib->addNode("Varname", "Misc", VarName);
    
    FRG::lib->addNode("Viewport", "Data", Viewport); 
    FRG::lib->addNode("Compose Object", "Data", ComposeObject); 
    FRG::lib->addNode("Compose Polygon", "Data", ComposePolygon); 
}
