#define GLM_FORCE_SWIZZLE
#include "skeleton.h"

using namespace MindTree;

PROPERTY_TYPE_INFO(JointPtr, "TRANSFORMABLE");

AbstractTransformablePtr Joint::clone() const
{
    return std::make_shared<Joint>(*this);
}

Joint::Joint()
    : AbstractTransformable(JOINT)
{
}

Joint::Joint(glm::vec3 pos, glm::vec3 x)
    : AbstractTransformable(JOINT)
{
    glm::vec4 worldPos = getWorldTransformation()[3];
    glm::vec3 y = glm::normalize(pos - worldPos.xyz());
    glm::vec3 z = glm::cross(y, x);

    setTransformation(glm::mat4(glm::vec4(x, 0),
                                glm::vec4(y, 0),
                                glm::vec4(z, 0),
                                glm::vec4(pos, 1)));
}
