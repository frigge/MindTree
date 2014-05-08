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
#ifndef NODETYPE_QDRSY1S8

#define NODETYPE_QDRSY1S8

#include "data/type.h"

namespace MindTree{

class NodeType;
namespace IO {
    void write(std::ostream &, const NodeType&);
}

class NodeType : public Type<NodeType> 
{
public:
    NodeType(const std::string &typeStr="")
        : Type<NodeType>(typeStr)
    {}

    NodeType(const char *typeStr)
        : Type<NodeType>(typeStr)
    {}

};

};

#endif /* end of include guard: NODETYPE_QDRSY1S8 */
