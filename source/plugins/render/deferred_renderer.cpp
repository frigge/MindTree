#define GLM_SWIZZLE
#include "render.h"
#include "renderpass.h"
#include "rendertree.h"
#include "../3dwidgets/widgets.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "functional"

#include "data/debuglog.h"
#include "light_renderer.h"
#include "pixel_plane.h"
#include "../datatypes/Object/lights.h"
#include "compositor_plane.h"
#include "shader_render_node.h"
#include "screenspace_reflection.h"
#include "rsm_computation_plane.h"
#include "deferred_light_block.h"
#include "gbuffer_block.h"
#include "benchmark.h"

#include "deferred_renderer.h"

using namespace MindTree;
using namespace GL;



DeferredRenderer::DeferredRenderer(QGLContext *context, CameraPtr camera, Widget3DManager *widgetManager) :
    RenderConfigurator(context, camera)
{
    auto manager = getManager();
    //auto benchmark = std::make_shared<Benchmark>("Render Benchmark(Overall)");
    //manager->setBenchmark(benchmark);
    //benchmark->setCallback([this](Benchmark* benchmark) {
    //                           if(benchmark->getNumCalls() >= 500) {
    //                               int vcnt = this->getVertexCount();
    //                               int pcnt = this->getPolygonCount();
    //                               std::cout << "======START======\n";
    //                               std::cout << "Vertexcount: " << vcnt;
    //                               std::cout << " Polygoncount: " << pcnt << "\n";
    //                               std::cout << (*benchmark) << "\n";
    //                               std::cout << "======END======" << std::endl;
    //                               benchmark->reset();
    //                           }
    //                       });

    auto rsm_generation_block = std::make_unique<RSMGenerationBlock>();
    auto gbuffer_block = std::make_unique<GBufferRenderBlock>(_geometryPass);
    auto gbuffer = gbuffer_block.get();
    auto rsm_generation = rsm_generation_block.get();
    addRenderBlock(std::move(gbuffer_block));
    addRenderBlock(std::move(rsm_generation_block));
    auto deferred_block = std::make_unique<DeferredLightingRenderBlock>(rsm_generation);
    auto deferred = deferred_block.get();
    addRenderBlock(std::move(deferred_block));
    auto rsm_block = std::make_unique<RSMEvaluationBlock>(rsm_generation);
    auto rsm = rsm_block.get();
    addRenderBlock(std::move(rsm_block));

    //auto ssreflection = std::make_shared<ScreenSpaceReflectionBlock>();
    //addRenderBlock(ssreflection);

    auto overlayPass = std::make_unique<RenderPass>("overlay");
    _overlayPass = overlayPass.get();
    manager->addPass(std::move(overlayPass));
    _overlayPass
        ->setDepthOutput(make_resource<Renderbuffer>(manager->getResourceManager(),
                                                     "depth",
                                                     Renderbuffer::DEPTH));
    auto overtxptr = make_resource<Texture2D>(manager->getResourceManager(),
                                           "overlay");
    auto overtx = overtxptr.get();
    _overlayPass->addOutput(std::move(overtxptr));
    _overlayPass->setCamera(camera);
    _viewCenter = new SinglePointRenderer();
    _viewCenter->setVisible(false);
    _overlayPass->addRenderer(_viewCenter);

    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(_overlayPass);

    auto compositor = std::make_unique<Compositor>();
    compositor->addLayer(deferred->getOutputs()[0], 1.0, CompositorPlane::CompositType::ALPHAOVER);
    //compositor->addLayer(ssreflection->getOutputs()[0], 1.0, CompositorPlane::CompositType::ADD);
    compositor->addLayer(rsm->getOutputs()[0], 1.0, CompositorPlane::CompositType::ADD);
    compositor->addLayer(gbuffer->getOutputs()[1], 1.0, CompositorPlane::CompositType::ALPHAOVER);
    compositor->addLayer(overtx, 1.0, CompositorPlane::CompositType::ALPHAOVER);
    addRenderBlock(std::move(compositor));

    auto finalPass = std::make_unique<RenderPass>("final");
    _finalPass = finalPass.get();
    finalPass->setCamera(camera);
    manager->addPass(std::move(finalPass));
    _finalPass->setCustomTextureNameMapping("final_out", "output");
    _finalPass->addRenderer(new PixelPlane("../plugins/render/defaultShaders/finalout.frag"));

    setCamera(camera);
}

void DeferredRenderer::setCamera(std::shared_ptr<Camera> cam)
{
    RenderConfigurator::setCamera(cam);
    _overlayPass->setCamera(cam);
    _finalPass->setCamera(cam);
}

glm::vec4 DeferredRenderer::getPosition(glm::vec2 pixel) const
{
    std::vector<std::string> values = {"worldposition"};
    return _geometryPass->readPixel(values, pixel)[0];
}

void DeferredRenderer::setProperty(const std::string &name, const Property &prop)
{
    RenderConfigurator::setProperty(name, prop);

    if (name == "GL:showgrid") {
        auto value = prop.getData<bool>();
        _grid->setVisible(value);
    }
    if (name == "GL:camera:showcenter") {
        auto value = prop.getData<bool>();
        _viewCenter->setVisible(value);
    }
    if (name == "GL:camera:center") {
        auto value = prop.getData<glm::vec3>();
        _viewCenter->setPosition(value);
    }
    if (name == "GL:backgroundColor") {
        auto value = prop.getData<glm::vec4>();
        _finalPass->setBackgroundColor(value);
    }
}

void DeferredRenderer::setOverrideOutput(std::string output)
{
    _finalPass->clearCustomTextureNameMapping();
    _finalPass->setCustomTextureNameMapping(output, "output");
    dbout("outputting: " << output);
}

void DeferredRenderer::clearOverrideOutput()
{
    _finalPass->clearCustomTextureNameMapping();
    _finalPass->setCustomTextureNameMapping("final_out", "output");
}
