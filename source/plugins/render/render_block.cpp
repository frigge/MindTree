#include "glm/gtc/matrix_transform.hpp"
#include "primitive_renderer.h"
#include "render_block.h"

using namespace MindTree;
using namespace GL;

RenderBlock::RenderBlock()
{

}

void RenderBlock::setGeometry()
{

}

void RenderBlock::setCamera(std::shared_ptr<Camera> camera)
{

}

void RenderBlock::addRendererFromTransformable(std::shared_ptr<AbstractTransformable> transformable)
{

}

void RenderBlock::addRendererFromObject(std::shared_ptr<GeoObject> obj)
{

}

void RenderBlock::addRendererFromLight(std::shared_ptr<Light> obj)
{

}

void RenderBlock::addRendererFromCamera(std::shared_ptr<Camera> obj)
{

}

void RenderBlock::addRendererFromEmpty(std::shared_ptr<Empty> obj)
{

}
