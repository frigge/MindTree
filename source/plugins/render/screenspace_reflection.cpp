#include "pixel_plane.h"
#include "renderpass.h"
#include "render_setup.h"
#include "rendertree.h"
#include "screenspace_reflection.h"

using namespace MindTree;
using namespace MindTree::GL;

void ScreenSpaceReflectionBlock::init()
{
    auto pass = addPass("ssreflection");
    auto reflection = make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                               "reflection",
                                               Texture::RGBA);
    auto reflection_dir = make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                   "reflection_dir",
                                                   Texture::RGBA16F);

    auto plane = new PixelPlane();
    plane->setFragmentShader("../plugins/render/defaultShaders/screenspace_reflection.frag");
    pass->addRenderer(plane);
    pass->addOutput(std::move(reflection));
    addOutput(reflection.get());
    pass->addOutput(std::move(reflection_dir));
}
