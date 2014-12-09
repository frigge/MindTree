#include "../datatypes/Object/object.h"

#include "empty_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

EmptyRenderer::EmptyRenderer(const EmptyPtr e)
    : _empty(e)
{
}

EmptyRenderer::~EmptyRenderer()
{
}
