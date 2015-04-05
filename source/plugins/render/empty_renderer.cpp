#include "../datatypes/Object/object.h"

#include "empty_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

EmptyRenderer::EmptyRenderer(const EmptyPtr e)
    : _empty(e)
{
    auto point = new SinglePointRenderer(this);
    point->setFillColor(glm::vec4(.5, 1., .5, 1.));
    point->setPointSize(5);

    setTransformation(e->getWorldTransformation());
}

EmptyRenderer::~EmptyRenderer()
{
}
