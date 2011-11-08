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
    //Standard starting nodes:
    //Array
    DNode* getArray(DNSpace *space);
    DNode* setArray(DNSpace *space);
    DNode* VarName(DNSpace *space);

    //Inputs
    DNode* surfaceInput(DNSpace *space);
    DNode* displacementInput(DNSpace *space);
    DNode* lightInput(DNSpace *space);
    DNode* volumeInput(DNSpace *space);
    //Outputs
    DNode* surfaceOutput(DNSpace *space);
    DNode* displacementOutput(DNSpace *space);
    DNode* volumeOutput(DNSpace *space);
    DNode* lightOutput(DNSpace *space);

    //Math
    DNode* MaddNode(DNSpace *space);
    DNode* MSubNode(DNSpace *space);
    DNode* MmultNode(DNSpace *space);
    DNode* MdivNode(DNSpace *space);
    DNode* MdotNode(DNSpace *space);

    //Booleans
    DNode* CandNode(DNSpace *space);
    DNode* CorNode(DNSpace *space);
    DNode* CnotNode(DNSpace *space);
    DNode* CgreaterNode(DNSpace *space);
    DNode* CsmallerNode(DNSpace *space);
    DNode* CeqNode(DNSpace *space);

    //Values
    DNode* VColNode(DNSpace *space);
    DNode* VStrNode(DNSpace *space);
    DNode* VFlNode(DNSpace *space);
    DNode* VVecNode(DNSpace *space);
//    void VMatNode(DNSpace *space);

    //Containers
    DNode* ContIfNode(DNSpace *space);
    DNode* ContForNode(DNSpace *space);
    DNode* ContWhileNode(DNSpace *space);

    //RSL Loop Container
    DNode* CLilluminate(DNSpace *space);
    DNode* CLilluminance(DNSpace *space);
    DNode* CLsolar(DNSpace *space);
    DNode* CLgather(DNSpace *space);

    //Data
    DNode *Viewport(DNSpace *space);
}
#endif // BUILDIN_NODES_H
