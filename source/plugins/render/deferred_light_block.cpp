#include "glwrapper.h"
#include "rendertree.h"
#include "render_setup.h"
#include "light_accumulation_plane.h"
#include "shadow_mapping.h"
#include "benchmark.h"

#include "deferred_light_block.h"

using namespace MindTree;
using namespace MindTree::GL;

DeferredLightingRenderBlock::DeferredLightingRenderBlock(ShadowMappingRenderBlock *shadowBlock) :
   _shadowBlock(shadowBlock)
{
}

void DeferredLightingRenderBlock::setProperty(std::string name, Property prop)
{
    RenderBlock::setProperty(name, prop);
    if(name == "GL:defaultLighting") {
        if(prop.getData<bool>()) {
            _deferredRenderer->setLights(_defaultLights);
        }
        else {
            _deferredRenderer->setLights(_sceneLights);
        }
    }
}

void DeferredLightingRenderBlock::setGeometry(std::shared_ptr<Group> grp)
{
    _sceneLights = grp->getLights();

    if(hasProperty("GL:defaultLighting") && !getProperty("GL:defaultLighting").getData<bool>())
        _deferredRenderer->setLights(_sceneLights);
    else
        _deferredRenderer->setLights(_defaultLights);

    setRenderersFromGroup(grp);
    if(_shadowBlock)
        _deferredRenderer->setShadowPasses(_shadowBlock->getShadowPasses());
}

glm::mat4 createTransFromZVec(glm::vec3 z)
{
    glm::mat4 trans;
    z = glm::normalize(z);
    glm::vec3 x = glm::normalize(glm::cross(z + glm::vec3(1, 0, 0), z));
    glm::vec3 y = glm::normalize(glm::cross(x, z));

    trans[0] = glm::vec4(x, 0);
    trans[1] = glm::vec4(y, 0);
    trans[2] = glm::vec4(z, 0);

    return trans;
}

void DeferredLightingRenderBlock::init()
{
    auto deferredPass = addPass();
    _deferredPass = deferredPass;
    deferredPass
        ->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                             "shading_out",
                                             Texture::RGBA16F));
    _deferredRenderer = new LightAccumulationPlane();
    deferredPass->addRenderer(_deferredRenderer);
    deferredPass->setBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    setupDefaultLights();

    setBenchmark(std::make_shared<Benchmark>("Deferred Shading"));

    for(auto tx : deferredPass->getOutputTextures())
        addOutput(tx);
}

void DeferredLightingRenderBlock::setupDefaultLights()
{
    static const double coneangle = 2 * 3.14159265359;
    auto light1 = std::make_shared<DistantLight>(.8, glm::vec4(1));
    light1->setTransformation(createTransFromZVec(glm::vec3(-1, -1, -1)));

    auto light2 = std::make_shared<DistantLight>(.3, glm::vec4(1));
    light2->setTransformation(createTransFromZVec(glm::vec3(5, 1, -1)));

    auto light3 = std::make_shared<DistantLight>(.1, glm::vec4(1));
    light3->setTransformation(createTransFromZVec(glm::vec3(0, 0, -1)));

    _defaultLights = {light1, light2, light3};
}
