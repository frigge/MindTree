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

#include "QGraphicsScene"
namespace BuildIn
{
    //Standard starting nodes:
    //Inputs
    void surfaceInput(QGraphicsScene *scene);
    void displacementInput(QGraphicsScene *scene);
    void lightInput(QGraphicsScene *scene);
    void volumeInput(QGraphicsScene *scene);
    //Outputs
    void surfaceOutput(QGraphicsScene *scene);
    void displacementOutput(QGraphicsScene *scene);
    void volumeOutput(QGraphicsScene *scene);
    void lightOutput(QGraphicsScene *scene);

    //Math
    void MaddNode(QGraphicsScene *scene);
    void MSubNode(QGraphicsScene *scene);
    void MmultNode(QGraphicsScene *scene);
    void MdivNode(QGraphicsScene *scene);
    void MdotNode(QGraphicsScene *scene);

    //Booleans
    void CandNode(QGraphicsScene *scene);
    void CorNode(QGraphicsScene *scene);
    void CnotNode(QGraphicsScene *scene);
    void CgreaterNode(QGraphicsScene *scene);
    void CsmallerNode(QGraphicsScene *scene);
    void CeqNode(QGraphicsScene *scene);

    //Values
    void VColNode(QGraphicsScene *scene);
    void VStrNode(QGraphicsScene *scene);
    void VFlNode(QGraphicsScene *scene);
//    void VVecNode(QGraphicsScene *scene);
//    void VMatNode(QGraphicsScene *scene);

    //Containers
    void ContIfNode(QGraphicsScene *scene);
    void ContForNode(QGraphicsScene *scene);
    void ContWhileNode(QGraphicsScene *scene);
    //void ContRSLLoopNode(QGraphicsScene *scene);
}
#endif // BUILDIN_NODES_H
