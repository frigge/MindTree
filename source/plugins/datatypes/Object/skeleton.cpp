#define GLM_SWIZZLE
#include "skeleton.h"

using namespace MindTree;

Skeleton::Skeleton() :
    AbstractTransformable(SKELETON)
{
}

void Skeleton::addJoint(Joint *parent, glm::vec3 pos, glm::vec3 right)
{
    parent->addChild(std::make_unique<Joint>(pos, right));
}

Skeleton::Joint::Joint(glm::vec3 pos, glm::vec3 x)
{
    glm::vec4 worldPos = worldTransform_[3];
    glm::vec3 y = glm::normalize(pos - worldPos.xyz());
    glm::vec3 z = glm::cross(y, x);

    localTransform_ = glm::mat4(glm::vec4(x, 0),
                                glm::vec4(y, 0),
                                glm::vec4(z, 0),
                                glm::vec4(pos, 1));

    worldTransform_ = localTransform_;
}

void Skeleton::Joint::addChild(std::unique_ptr<Joint> &&joint)
{
    joint->setParent(this);
    children_.push_back(std::move(joint));
}

std::vector<const Skeleton::Joint*> Skeleton::Joint::getChildren() const
{
    std::vector<const Skeleton::Joint*> ret;
    for(const auto &joint : children_)
        ret.push_back(joint.get());

    return ret;
}

void Skeleton::Joint::setParent(Joint *joint)
{
    parent_ = joint;
    cacheWorldTransform();

    //Joint should stay in place but now needs to be represented relative
    //to its parent
    localTransform_ = glm::inverse(parent_->worldTransform_)
        * localTransform_;
}

const Skeleton::Joint* Skeleton::getRoot() const
{
    return root_.get();
}

glm::mat4 Skeleton::Joint::getWorldTransform() const
{
    return worldTransform_;
}

glm::mat4 Skeleton::Joint::cacheWorldTransform()
{
    if(parent_)
        worldTransform_ = parent_->worldTransform_ * localTransform_;
    else
        worldTransform_ = localTransform_;
}
