#ifndef MT_GL_SKELETON_RENDERER_H
#define MT_GL_SKELETON_RENDERER_H

#include "primitive_renderer.h"

namespace MindTree {
class Skeleton;

namespace GL {

class SkeletonRenderer : public ShapeRendererGroup
{
public:
    SkeletonRenderer(Skeleton *skel, ShapeRendererGroup *parent=nullptr);

private:
    Skeleton *skeleton_;
};

}
}
#endif
