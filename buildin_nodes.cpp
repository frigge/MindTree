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


#include "node.h"
#include "buildin_nodes.h"
#include "shader_space.h"

void    BuildIn::surfaceInput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    InputNode *surfnode = new InputNode();
    Node::setsurfaceInput(surfnode);
    surfnode->setPos(space->mousePos);
    space->addNode(surfnode);
};

void BuildIn::displacementInput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    InputNode *dispnode = new InputNode();
    Node::setdisplacementInput(dispnode);
    dispnode->setPos(space->mousePos);
    space->addNode(dispnode);
};

void BuildIn::volumeInput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    InputNode *volnode = new InputNode();
    Node::setvolumeInput(volnode);
    volnode->setPos(space->mousePos);
    space->addNode(volnode);
};

void BuildIn::lightInput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    InputNode *lnode = new InputNode();
    Node::setlightInput(lnode);
    lnode->setPos(space->mousePos);
    space->addNode(lnode);
};

void BuildIn::surfaceOutput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    OutputNode *surfnode = new OutputNode();
    Node::setsurfaceOutput(surfnode);
    surfnode->setPos(space->mousePos);
    space->addNode(surfnode);
};

void BuildIn::displacementOutput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    OutputNode *dispnode = new OutputNode();
    Node::setdisplacementOutput(dispnode);
    dispnode->setPos(space->mousePos);
    space->addNode(dispnode);
}

void BuildIn::volumeOutput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    OutputNode *volnode = new OutputNode();
    Node::setvolumeOutput(volnode);
    volnode->setPos(space->mousePos);
    space->addNode(volnode);
}

void BuildIn::lightOutput(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    OutputNode *lnode = new OutputNode();
    Node::setlightOutput(lnode);
    lnode->setPos(space->mousePos);
    space->addNode(lnode);
}

void BuildIn::MaddNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    MathNode *node = new MathNode(ADD);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::MSubNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    MathNode *node = new MathNode(SUBTRACT);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::MmultNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    MathNode *node = new MathNode(MULTIPLY);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::MdivNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    MathNode *node = new MathNode(DIVIDE);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::MdotNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    MathNode *node = new MathNode(DOTPRODUCT);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CandNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionNode *node = new ConditionNode(AND);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CorNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionNode *node = new ConditionNode(OR);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CnotNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionNode *node = new ConditionNode(NOT);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CgreaterNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionNode *node = new ConditionNode(GREATERTHAN);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CsmallerNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionNode *node = new ConditionNode(SMALLERTHAN);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CeqNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionNode *node = new ConditionNode(EQUAL);
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::VColNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ColorValueNode *node = new ColorValueNode;
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::VStrNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    StringValueNode *node = new StringValueNode;
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::VFlNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    FloatValueNode *node = new FloatValueNode;
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::ContIfNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ConditionContainerNode *node = new ConditionContainerNode;
    node->setNodeName("Boolean");
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::ContForNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    ForNode *node = new ForNode(false);
    node->setNodeName("For");
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::ContWhileNode(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    WhileNode *node = new WhileNode(false);
    node->setNodeName("While");
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CLilluminate(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    IlluminateNode *node = new IlluminateNode(false);
    node->setNodeName("Illuminate");
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CLgather(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    GatherNode *node = new GatherNode(false);
    node->setNodeName("Gather");
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CLilluminance(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    IlluminanceNode *node = new IlluminanceNode(false);
    node->setNodeName("Illuminance");
    node->setPos(space->mousePos);
    space->addNode(node);
}

void BuildIn::CLsolar(QGraphicsScene *scene)
{
    Shader_Space *space = (Shader_Space*)scene;
    SolarNode *node = new SolarNode(false);
    node->setNodeName("Solar");
    node->setPos(space->mousePos);
    space->addNode(node);
}
