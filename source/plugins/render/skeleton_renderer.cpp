#define GLM_SWIZZLE
#include "coordsystem_renderer.h"
#include "skeleton_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

SkeletonRenderer::SkeletonRenderer(JointPtr skel, ShapeRendererGroup *parent) :
    ShapeRendererGroup(parent), skeleton_(skel)
{
    auto *coord = new CoordSystemRenderer(this);
    if(skel->getParent() && skel->getParent()->getType() == AbstractTransformable::JOINT) {
        auto *line = new LineRenderer({skel->getParent()->getWorldTransformation()[3].xyz(),
                    skel->getWorldTransformation()[3].xyz()});
            line->setParentPrimitive(this);
    }
}
