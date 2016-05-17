#ifndef MT_SKELETON_H
#define MT_SKELETON_H

#include "memory"

#include "object.h"

namespace MindTree {

class Skeleton : public AbstractTransformable
{
public:
    class Joint
    {
    public:
        Joint(glm::vec3 pos, glm::vec3 x);
        void setParent(Joint *joint);
        void addChild(std::unique_ptr<Joint> &&joint);

        std::vector<const Skeleton::Joint*> getChildren() const;

        glm::mat4 getWorldTransform() const;

    private:
        glm::mat4 cacheWorldTransform();

        Joint *parent_{nullptr};
        std::vector<std::unique_ptr<Joint>> children_;

        glm::mat4 localTransform_;
        glm::mat4 worldTransform_;
        glm::mat4 skinTransform_;
    };

    Skeleton();

    void addJoint(Joint *parent, glm::vec3 pos, glm::vec3 right);
    const Joint* getRoot() const;

private:
    std::unique_ptr<Joint> root_;
};

}

#endif
