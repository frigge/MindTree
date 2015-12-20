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
#include "rsm_computation_plane.h"
#include "deferred_light_block.h"
#include "gbuffer_block.h"
#include "benchmark.h"

#include "deferred_renderer.h"

using namespace MindTree;
using namespace GL;

struct FinalOut;
template<>
const std::string
PixelPlane::ShaderFiles<FinalOut>::
fragmentShader = "../plugins/render/defaultShaders/finalout.frag";

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

    auto rsmGenerationBlock = std::make_shared<RSMGenerationBlock>();
    auto gbuffer = std::make_shared<GBufferRenderBlock>(_geometryPass);
    addRenderBlock(gbuffer);
    addRenderBlock(rsmGenerationBlock);
    auto deferred = std::make_shared<DeferredLightingRenderBlock>(rsmGenerationBlock.get());
    addRenderBlock(deferred);
    auto rsm = std::make_shared<RSMEvaluationBlock>(rsmGenerationBlock.get());
    addRenderBlock(rsm);

    auto overlayPass = std::make_shared<RenderPass>();
    _overlayPass = overlayPass;
    manager->addPass(overlayPass);
    auto overlay = _overlayPass.lock();
    overlay
        ->setDepthOutput(std::make_shared<Renderbuffer>("depth",
                                                        Renderbuffer::DEPTH));
    auto overtx = std::make_shared<Texture2D>("overlay");
    overlay->addOutput(overtx);
    overlay->setCamera(camera);
    _viewCenter = new SinglePointRenderer();
    _viewCenter->setVisible(false);
    overlay->addRenderer(_viewCenter);

    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(overlay);

    auto compositor = std::make_shared<Compositor>();
    compositor->addLayer(deferred->getOutputs()[0], 1.0, CompositorPlane::CompositType::ALPHAOVER);
    compositor->addLayer(rsm->getOutputs()[0], 1.0, CompositorPlane::CompositType::ALPHAOVER);
    compositor->addLayer(gbuffer->getOutputs()[1], 1.0, CompositorPlane::CompositType::ALPHAOVER);
    compositor->addLayer(overtx, 1.0, CompositorPlane::CompositType::ALPHAOVER);
    addRenderBlock(compositor);

    auto finalPass = std::make_shared<RenderPass>();
    _finalPass = finalPass;
    finalPass->setCamera(camera);
    manager->addPass(finalPass);
    finalPass->setCustomTextureNameMapping("final_out", "output");
    PixelPlane *finalPlane = new PixelPlane();
    finalPlane->setProvider<FinalOut>();
    finalPass->addRenderer(finalPlane);

    setCamera(camera);
}

void DeferredRenderer::setCamera(std::shared_ptr<Camera> cam)
{
    RenderConfigurator::setCamera(cam);
    _overlayPass.lock()->setCamera(cam);
    _finalPass.lock()->setCamera(cam);
}

glm::vec4 DeferredRenderer::getPosition(glm::vec2 pixel) const
{
    std::vector<std::string> values = {"worldposition"};
    return _geometryPass.lock()->readPixel(values, pixel)[0];
}

void DeferredRenderer::setProperty(std::string name, Property prop)
{
    RenderConfigurator::setProperty(name, prop);

    dbout(name);
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
        _finalPass.lock()->setBackgroundColor(value);
    }
}

void DeferredRenderer::setOverrideOutput(std::string output)
{
    _finalPass.lock()->clearCustomTextureNameMapping();
    _finalPass.lock()->setCustomTextureNameMapping(output, "output");
    dbout("outputting: " << output);
}

void DeferredRenderer::clearOverrideOutput()
{
    _finalPass.lock()->clearCustomTextureNameMapping();
    _finalPass.lock()->setCustomTextureNameMapping("final_out", "output");
}
