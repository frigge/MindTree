#define GLM_SWIZZLE
#include "../datatypes/Object/skeleton.h"
#include "skeleton_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

SkeletonRenderer::SkeletonRenderer(Joint *skel, ShapeRendererGroup *parent) :
    ShapeRendererGroup(parent), skeleton_(skel)
{
    std::stack<const Joint*> joints;
    joints.push(skel);

    while(!joints.empty()) {
        const auto *joint = joints.top();
        joints.pop();

        glm::vec3 start = joint->getWorldTransform()[3].xyz();
        auto children = joint->getChildren();
        for(const auto *child : joint->getChildren()) {
            joints.push(child);
            auto *line = new LineRenderer({start, child->getWorldTransform()[3].xyz()});
            line->setParentPrimitive(this);
        }
    }
}
