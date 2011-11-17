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
    void* getArray(void *space);
    void* setArray(void *space);
    void* composeArray(void *space);
    void* VarName(void *space);

    //Inputs
    void* surfaceInput(void *space);
    void* displacementInput(void *space);
    void* lightInput(void *space);
    void* volumeInput(void *space);
    //Outputs
    void* surfaceOutput(void *space);
    void* displacementOutput(void *space);
    void* volumeOutput(void *space);
    void* lightOutput(void *space);

    //Math
    void* MaddNode(void *space);
    void* MSubNode(void *space);
    void* MmultNode(void *space);
    void* MdivNode(void *space);
    void* MdotNode(void *space);

    //Booleans
    void* CandNode(void *space);
    void* CorNode(void *space);
    void* CnotNode(void *space);
    void* CgreaterNode(void *space);
    void* CsmallerNode(void *space);
    void* CeqNode(void *space);

    //Values
    void* VColNode(void *space);
    void* VStrNode(void *space);
    void* VFlNode(void *space);
    void* VIntNode(void *space);
    void* VBoolNode(void *space);
    void* VVecNode(void *space);
//    void VMatNode(void *space);

    //Containers
    void* ContIfNode(void *space);
    void* ContForNode(void *space);
    void* ContWhileNode(void *space);

    //RSL Loop Container
    void* CLilluminate(void *space);
    void* CLilluminance(void *space);
    void* CLsolar(void *space);
    void* CLgather(void *space);

    //Data
    void* Viewport(void *space);
    void* ComposeObject(void *space);
    void* ComposePolygon(void *space);
}
#endif // BUILDIN_NODES_H
