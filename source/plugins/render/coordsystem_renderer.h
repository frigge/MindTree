#ifndef MT_RENDER_COORDSYSTEM_H
#define MT_RENDER_COORDSYSTEM_H

#include "primitive_renderer.h"

namespace MindTree {
namespace GL {

class CoordSystemRenderer : public ShapeRendererGroup
{
 public:
    CoordSystemRenderer(ShapeRendererGroup *parent=nullptr);
};

}
}
#endif
