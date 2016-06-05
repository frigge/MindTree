#ifndef MT_SKELETON_H
#define MT_SKELETON_H

#include "memory"

#include "object.h"

namespace MindTree {

class Joint : public AbstractTransformable
{
public:
    Joint();
    Joint(glm::vec3 pos, glm::vec3 x);

    AbstractTransformablePtr clone() const override;

private:
    glm::mat4 cacheWorldTransform();
    glm::mat4 worldTransform_;
    glm::mat4 skinTransform_;
};

 typedef std::shared_ptr<Joint> JointPtr;

}

#endif
