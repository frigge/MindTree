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
