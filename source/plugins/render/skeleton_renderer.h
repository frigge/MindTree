#ifndef MT_GL_SKELETON_RENDERER_H
#define MT_GL_SKELETON_RENDERER_H

#include "../datatypes/Object/skeleton.h"
#include "primitive_renderer.h"

namespace MindTree {

namespace GL {

class SkeletonRenderer : public ShapeRendererGroup
{
public:
    SkeletonRenderer(JointPtr skel, ShapeRendererGroup *parent=nullptr);

private:
    JointPtr skeleton_;
};

}
}
#endif
