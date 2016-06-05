#define GLM_SWIZZLE
#include "skeleton_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

SkeletonRenderer::SkeletonRenderer(JointPtr skel, ShapeRendererGroup *parent) :
    ShapeRendererGroup(parent), skeleton_(skel)
{
    std::stack<Joint*> joints;
    joints.push(skel.get());

    while(!joints.empty()) {
        const auto *joint = joints.top();
        joints.pop();

        glm::vec3 start = joint->getWorldTransformation()[3].xyz();
        auto children = joint->getChildren();
        for(const auto child : joint->getChildren()) {
            if(!child->getType() == AbstractTransformable::JOINT)
                continue;
            joints.push(static_cast<Joint*>(child.get()));
            auto *line = new LineRenderer({start, child->getWorldTransformation()[3].xyz()});
            line->setParentPrimitive(this);
        }
    }
}
