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

#ifndef MATH_AHZJP9OE

#define MATH_AHZJP9OE

#include "data/nodes/data_node.h"


class MathNode : public MindTree::DNode
{
public:
    enum mathType {
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        DOTPRODUCT,
        MODULO
    };

    MathNode(MindTree::NodeType t, bool raw=false);
    MathNode(const MathNode &node);

    virtual void decVarSocket(MindTree::DSocket *socket);
    virtual void incVarSocket();
};

#endif /* end of include guard: MATH_AHZJP9OE */
