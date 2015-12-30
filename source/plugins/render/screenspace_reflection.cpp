#include "pixel_plane.h"
#include "renderpass.h"
#include "screenspace_reflection.h"

using namespace MindTree;
using namespace MindTree::GL;

template<>
const std::string
PixelPlane::ShaderFiles<ScreenSpaceReflectionBlock>::
fragmentShader = "../plugins/render/defaultShaders/screenspace_reflection.frag";

void ScreenSpaceReflectionBlock::init()
{
    auto pass = addPass();
    auto reflection = std::make_shared<Texture2D>("reflection", Texture::RGBA);

    auto plane = new PixelPlane();
    plane->setProvider<ScreenSpaceReflectionBlock>();
    pass->addRenderer(plane);
    pass->addOutput(reflection);
    addOutput(reflection);
}
