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
#include "light_accumulation_plane.h"
#include "pixel_plane.h"
#include "../datatypes/Object/lights.h"
#include "polygon_renderer.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "shader_render_node.h"
#include "rsm_computation_plane.h"
#include "benchmark.h"
#include "ambient_occlusion.h"

#include "deferred_renderer.h"

using namespace MindTree;
using namespace GL;

struct Compositor : public PixelPlane::ShaderProvider {
    std::shared_ptr<ShaderProgram> provideProgram() {
        auto prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.frag", 
                                ShaderProgram::FRAGMENT);

        return prog;
    }
};

struct FinalOut : public PixelPlane::ShaderProvider {
    std::shared_ptr<ShaderProgram> provideProgram() {
        auto prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/finalout.frag", 
                                ShaderProgram::FRAGMENT);

        return prog;
    }
};

DeferredRenderer::DeferredRenderer(QGLContext *context, CameraPtr camera, Widget3DManager *widgetManager) :
    RenderConfigurator(context, camera)
{
    auto manager = getManager();
    auto benchmark = std::make_shared<Benchmark>("Render Benchmark(Overall)");
    manager->setBenchmark(benchmark);
//    benchmark->setCallback([this](Benchmark* benchmark) {
//                               if(benchmark->getNumCalls() >= 100) {
//                                   int vcnt = this->getVertexCount();
//                                   int pcnt = this->getPolygonCount();
//                                   std::cout << "======START======\n";
//                                   std::cout << "Vertexcount: " << vcnt;
//                                   std::cout << " Polygoncount: " << pcnt << "\n";
//                                   std::cout << (*benchmark) << "\n";
//                                   std::cout << "======END======" << std::endl;
//                                   benchmark->reset();
//                               }
//                           });
//
    auto rsmGenerationBlock = std::make_shared<RSMGenerationBlock>();
    addRenderBlock(std::make_shared<GBufferRenderBlock>(_geometryPass));
    addRenderBlock(rsmGenerationBlock);
    addRenderBlock(std::make_shared<DeferredLightingRenderBlock>(rsmGenerationBlock.get()));
    addRenderBlock(std::make_shared<RSMEvaluationBlock>(rsmGenerationBlock.get()));
    //addRenderBlock(std::make_shared<AmbientOcclusionBlock>());

    auto overlayPass = std::make_shared<RenderPass>();
    _overlayPass = overlayPass;
    manager->addPass(overlayPass);
    auto overlay = _overlayPass.lock();
    overlay
        ->setDepthOutput(std::make_shared<Renderbuffer>("depth",
                                                        Renderbuffer::DEPTH));
    overlay->addOutput(std::make_shared<Texture2D>("overlay"));
    overlay->setCamera(camera);
    _viewCenter = new SinglePointRenderer();
    _viewCenter->setVisible(false);
    overlay->addRenderer(_viewCenter);

    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(overlay);

    auto pixelPass = std::make_shared<RenderPass>();
    _pixelPass = pixelPass;
    manager->addPass(pixelPass);
    pixelPass->setCamera(camera);
    auto pplane = new PixelPlane();
    pplane->setProvider<Compositor>();
    pixelPass->addRenderer(pplane);
    float value = 70.0 / 255;
    pixelPass->setBackgroundColor(glm::vec4(value, value, value, 1.));
    pixelPass->addOutput(std::make_shared<Texture2D>("final_out"));

    auto finalPass = std::make_shared<RenderPass>();
    _finalPass = finalPass;
    finalPass->setCamera(camera);
    manager->addPass(finalPass);
    finalPass->setCustomTextureNameMapping("final_out", "output");
    pplane = new PixelPlane();
    pplane->setProvider<FinalOut>();
    finalPass->addRenderer(pplane);

    setCamera(camera);
}

void DeferredRenderer::setCamera(std::shared_ptr<Camera> cam)
{
    RenderConfigurator::setCamera(cam);
    _overlayPass.lock()->setCamera(cam);
    _pixelPass.lock()->setCamera(cam);
}

glm::vec4 DeferredRenderer::getPosition(glm::vec2 pixel) const
{
    std::vector<std::string> values = {"worldposition"};
    return _geometryPass.lock()->readPixel(values, pixel)[0];
}

void DeferredRenderer::setProperty(std::string name, Property prop)
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

GBufferRenderBlock::GBufferRenderBlock(std::weak_ptr<RenderPass> geopass)
    : GeometryRenderBlock(geopass)
{
    auto gbBench = std::make_shared<Benchmark>("GBuffer Creation");
    geopass.lock()->setBenchmark(gbBench);
    setBenchmark(std::make_shared<Benchmark>("GBuffer"));
    getBenchmark().lock()->addBenchmark(gbBench);
}

GBufferRenderBlock::~GBufferRenderBlock()
{
}

void GBufferRenderBlock::init()
{
    auto geopass = _geometryPass.lock();

    geopass->setEnableBlending(false);

    geopass->setDepthOutput(std::make_shared<Texture2D>("depth", 
                                                        Texture::DEPTH));
    geopass->addOutput(std::make_shared<Texture2D>("outdiffusecolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outcolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outdiffuseintensity"));
    geopass->addOutput(std::make_shared<Texture2D>("outspecintensity"));
    auto normal = std::make_shared<Texture2D>("outnormal",
                                              Texture::RGBA16F);
    normal->generateMipmaps();
    geopass->addOutput(normal);
    auto position = std::make_shared<Texture2D>("outposition", 
                                                Texture::RGBA16F);
    position->generateMipmaps();
    geopass->addOutput(position);
    geopass->addOutput(std::make_shared<Texture2D>("worldposition", 
                                                   Texture::RGBA16F));

    geopass->addPostRenderCallback([normal, position] (RenderPass *) {
                                       normal->generateMipmaps();
                                       position->generateMipmaps();
                                   });

    setupGBuffer();
}

void GBufferRenderBlock::setupGBuffer()
{
    auto gbufferShader = std::make_shared<ShaderProgram>();
    gbufferShader
        ->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert", 
                            ShaderProgram::VERTEX);
    gbufferShader
        ->addShaderFromFile("../plugins/render/defaultShaders/gbuffer.frag", 
                            ShaderProgram::FRAGMENT);
    auto gbufferNode = std::make_shared<ShaderRenderNode>(gbufferShader);
    _gbufferNode = gbufferNode;

    _geometryPass.lock()->addGeometryShaderNode(gbufferNode);
}

void GBufferRenderBlock::addRendererFromObject(std::shared_ptr<GeoObject> obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            _gbufferNode->addRenderer(new PolygonRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new EdgeRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void GBufferRenderBlock::setProperty(std::string name, Property prop)
{
    if(name == "defaultLighting") {
        _geometryPass.lock()->setProperty(name, prop);
    }
}

DeferredLightingRenderBlock::DeferredLightingRenderBlock(ShadowMappingRenderBlock *shadowBlock) :
   _shadowBlock(shadowBlock)
{
}

void DeferredLightingRenderBlock::setProperty(std::string name, Property prop)
{
    RenderBlock::setProperty(name, prop);
    if(name == "defaultLighting") {
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

    if(hasProperty("defaultLighting") && !getProperty("defaultLighting").getData<bool>())
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
        ->addOutput(std::make_shared<Texture2D>("shading_out",
                                                Texture::RGB16F));
    _deferredRenderer = new LightAccumulationPlane();
    deferredPass->addRenderer(_deferredRenderer);
    deferredPass->setBlendFunc(GL_ONE, GL_ONE);

    setupDefaultLights();

    setBenchmark(std::make_shared<Benchmark>("Deferred Shading"));
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

