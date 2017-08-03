#define GLM_FORCE_SWIZZLE
#include "coordsystem_renderer.h"
#include "skeleton_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

SkeletonRenderer::SkeletonRenderer(JointPtr skel, ShapeRendererGroup *parent) :
    ShapeRendererGroup(parent), skeleton_(skel)
{
    std::stack<Joint*> joints;
    joints.push(skel.get());

    std::vector<glm::vec3> lines;
    while(!joints.empty()) {
        auto *j = joints.top();
        joints.pop();

        glm::vec3 start = j->getWorldTransformation()[3].xyz();

        for(const auto &child : j->getChildren()) {
            if(child->getType() != AbstractTransformable::JOINT)
                continue;

            glm::vec3 end = child->getWorldTransformation()[3].xyz();
            lines.push_back(start);
            lines.push_back(end);

            joints.push(static_cast<Joint*>(child.get()));
        }
    }

    auto *line = new LineRenderer();
    line->setPoints(lines);
    line->setParentPrimitive(this);
}
