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


#ifndef BUILDIN_NODES_H
#define BUILDIN_NODES_H

#include "source/data/base/dnspace.h"
#include "source/data/base/frg.h"

namespace BuildIn
{
    void registerNodes();
    template<class T> 
    DNode* createNode()
    {
        T* node = new T();
        FRG::SpaceDataInFocus->addNode(node);
        return node;
    }
    //
    //Math
    DNode* MaddNode();
    DNode* MSubNode();
    DNode* MmultNode();
    DNode* MdivNode();
    DNode* MdotNode();
    DNode* MmodNode();

    //Booleans
    DNode* CandNode();
    DNode* CorNode();
    DNode* CnotNode();
    DNode* CgreaterNode();
    DNode* CsmallerNode();
    DNode* CeqNode();
}
#endif // BUILDIN_NODES_H
