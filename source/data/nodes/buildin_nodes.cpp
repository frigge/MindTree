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

DNode* BuildIn::getArray(DNSpace *space)    
{
    GetArrayNode *node = new GetArrayNode();
    space->addNode(node);
    return node;
};

DNode* BuildIn::setArray(DNSpace *space)    
{
    SetArrayNode *node = new SetArrayNode();
    space->addNode(node);
    return node;
};

DNode* BuildIn::VarName(DNSpace *space)    
{
    VarNameNode *node = new VarNameNode();
    space->addNode(node);
    return node;
};

DNode* BuildIn::surfaceInput(DNSpace *space)
{
    InputNode *node = new InputNode();
    DNode::setsurfaceInput(node);
    space->addNode(node);
    return node;
};

DNode* BuildIn::displacementInput(DNSpace *space)
{
    InputNode *node = new InputNode();
    DNode::setdisplacementInput(node);
    space->addNode(node);
    return node;
};

DNode* BuildIn::volumeInput(DNSpace *space)
{
    InputNode *node = new InputNode();
    DNode::setvolumeInput(node);
    space->addNode(node);
    return node;
};

DNode* BuildIn::lightInput(DNSpace *space)
{
    InputNode *node = new InputNode();
    DNode::setlightInput(node);
    space->addNode(node);
    return node;
};

DNode* BuildIn::surfaceOutput(DNSpace *space)
{
    OutputNode *node = new OutputNode();
    DNode::setsurfaceOutput(node);
    space->addNode(node);
    return node;
};

DNode* BuildIn::displacementOutput(DNSpace *space)
{
    OutputNode *node = new OutputNode();
    DNode::setdisplacementOutput(node);
    space->addNode(node);
    return node;
}

DNode* BuildIn::volumeOutput(DNSpace *space)
{
    OutputNode *node = new OutputNode();
    DNode::setvolumeOutput(node);
    space->addNode(node);
    return node;
}

DNode* BuildIn::lightOutput(DNSpace *space)
{
    OutputNode *node = new OutputNode();
    DNode::setlightOutput(node);
    space->addNode(node);
    return node;
}

DNode* BuildIn::MaddNode(DNSpace *space)
{
    MathNode *node = new MathNode(ADD);
    space->addNode(node);
    return node;
}

DNode* BuildIn::MSubNode(DNSpace *space)
{
    MathNode *node = new MathNode(SUBTRACT);
    space->addNode(node);
    return node;
}

DNode* BuildIn::MmultNode(DNSpace *space)
{
    MathNode *node = new MathNode(MULTIPLY);
    space->addNode(node);
    return node;
}

DNode* BuildIn::MdivNode(DNSpace *space)
{
    MathNode *node = new MathNode(DIVIDE);
    space->addNode(node);
    return node;
}

DNode* BuildIn::MdotNode(DNSpace *space)
{
    MathNode *node = new MathNode(DOTPRODUCT);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CandNode(DNSpace *space)
{
    ConditionNode *node = new ConditionNode(AND);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CorNode(DNSpace *space)
{
    ConditionNode *node = new ConditionNode(OR);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CnotNode(DNSpace *space)
{
    ConditionNode *node = new ConditionNode(NOT);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CgreaterNode(DNSpace *space)
{
    ConditionNode *node = new ConditionNode(GREATERTHAN);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CsmallerNode(DNSpace *space)
{
    ConditionNode *node = new ConditionNode(SMALLERTHAN);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CeqNode(DNSpace *space)
{
    ConditionNode *node = new ConditionNode(EQUAL);
    space->addNode(node);
    return node;
}

DNode* BuildIn::VColNode(DNSpace *space)
{
    ColorValueNode *node = new ColorValueNode;
    space->addNode(node);
    return node;
}

DNode* BuildIn::VStrNode(DNSpace *space)
{
    StringValueNode *node = new StringValueNode;
    space->addNode(node);
    return node;
}

DNode* BuildIn::VFlNode(DNSpace *space)
{
    FloatValueNode *node = new FloatValueNode;
    space->addNode(node);
    return node;
}

DNode* BuildIn::VVecNode(DNSpace *space)
{
    VectorValueNode *node = new VectorValueNode;
    space->addNode(node);
    return node;
}

DNode* BuildIn::ContIfNode(DNSpace *space)
{
    ConditionContainerNode *node = new ConditionContainerNode;
    space->addNode(node);
    return node;
}

DNode* BuildIn::ContForNode(DNSpace *space)
{
    ForNode *node = new ForNode(false);
    space->addNode(node);
    return node;
}

DNode* BuildIn::ContWhileNode(DNSpace *space)
{
    WhileNode *node = new WhileNode(false);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CLilluminate(DNSpace *space)
{
    IlluminateNode *node = new IlluminateNode(false);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CLgather(DNSpace *space)
{
    GatherNode *node = new GatherNode(false);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CLilluminance(DNSpace *space)
{
    IlluminanceNode *node = new IlluminanceNode(false);
    space->addNode(node);
    return node;
}

DNode* BuildIn::CLsolar(DNSpace *space)
{
    SolarNode *node = new SolarNode(false);
    space->addNode(node);
    return node;
}

DNode* BuildIn::Viewport(DNSpace *space)
{
    ViewportNode *node = new ViewportNode(false);
    space->addNode(node);
    return node;
}
