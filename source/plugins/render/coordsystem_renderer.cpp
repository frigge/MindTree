#include "coordsystem_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

CoordSystemRenderer::CoordSystemRenderer(ShapeRendererGroup *parent)
    : ShapeRendererGroup(parent)
{
    auto *x = new LineRenderer({glm::vec3(0), glm::vec3(1, 0, 0)});
    auto *y = new LineRenderer({glm::vec3(0), glm::vec3(0, 1, 0)});
    auto *z = new LineRenderer({glm::vec3(0), glm::vec3(0, 0, 1)});

    x->setParentPrimitive(this);
    y->setParentPrimitive(this);
    z->setParentPrimitive(this);

    x->setBorderColor(glm::vec4(1, 0, 0, 1));
    y->setBorderColor(glm::vec4(0, 1, 0, 1));
    z->setBorderColor(glm::vec4(0, 0, 1, 1));
}
