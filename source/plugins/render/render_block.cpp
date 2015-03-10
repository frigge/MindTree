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

GridBlock::GridBlock(RenderTree *tree)
    : _tree(tree)
{
    auto grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    grid->setTransformation(trans);
    grid->setBorderColor(glm::vec4(.3, .3, .3, 1.0));
    grid->setAlternatingColor(glm::vec4(.7, .7, .7, 1.0));
    grid->setBorderWidth(2.);
}
