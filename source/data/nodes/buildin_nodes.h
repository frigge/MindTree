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

namespace BuildIn
{
    void registerNodes();
    //Standard starting nodes:
    //Array
    DNode* getArray();
    DNode* setArray();
    DNode* composeArray();
    DNode* VarName();

    //Inputs
    DNode* surfaceInput();
    DNode* displacementInput();
    DNode* lightInput();
    DNode* volumeInput();

    //Outputs
    DNode* surfaceOutput();
    DNode* displacementOutput();
    DNode* volumeOutput();
    DNode* lightOutput();
    DNode* preview();

    //Math
    DNode* MaddNode();
    DNode* MSubNode();
    DNode* MmultNode();
    DNode* MdivNode();
    DNode* MdotNode();

    //Booleans
    DNode* CandNode();
    DNode* CorNode();
    DNode* CnotNode();
    DNode* CgreaterNode();
    DNode* CsmallerNode();
    DNode* CeqNode();

    //Values
    DNode* VColNode();
    DNode* VStrNode();
    DNode* VFlNode();
    DNode* VIntNode();
    DNode* VBoolNode();
    DNode* VVecNode();
    DNode* VFloatToVector();
//    void VMatNode(void *space);

    //Containers
    DNode* ContIfNode();
    DNode* ContForNode();
    DNode* ContWhileNode();

    //RSL Loop Container
    DNode* CLilluminate();
    DNode* CLilluminance();
    DNode* CLsolar();
    DNode* CLgather();

    //Data
    DNode* Viewport();
    DNode* ComposeObject();
    DNode* ComposePolygon();
}
#endif // BUILDIN_NODES_H
