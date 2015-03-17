#define GLM_SWIZZLE
#include "render.h"
#include "renderpass.h"
#include "rendertree.h"
#include "../3dwidgets/widgets.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "data/debuglog.h"
#include "polygon_renderer.h"
#include "light_renderer.h"
#include "light_accumulation_plane.h"
#include "pixel_plane.h"
#include "../datatypes/Object/lights.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "shader_render_node.h"
#include "rsm_computation_plane.h"

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
    RenderTree *manager = getManager();
    auto config = manager->getConfig();
    config.setProperty("defaultLighting", true);
    manager->setConfig(config);

    _grid = new GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    _grid->setTransformation(trans);
    _grid->setBorderColor(glm::vec4(.3, .3, .3, 1.0));
    _grid->setAlternatingColor(glm::vec4(.7, .7, .7, 1.0));
    _grid->setBorderWidth(2.);

    auto geometryPass = std::make_shared<RenderPass>();
    _geometryPass = geometryPass;
    manager->addPass(geometryPass);
    auto geopass = _geometryPass.lock();

    geopass->setCamera(camera);
    geopass->setEnableBlending(false);

    geopass->setDepthOutput(std::make_shared<Texture2D>("depth", 
                                                        Texture::DEPTH));
    geopass->addOutput(std::make_shared<Texture2D>("outdiffusecolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outcolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outdiffuseintensity"));
    geopass->addOutput(std::make_shared<Texture2D>("outspecintensity"));
    geopass->addOutput(std::make_shared<Texture2D>("outnormal", 
                                                   Texture::RGBA16F));
    geopass->addOutput(std::make_shared<Texture2D>("outposition", 
                                                   Texture::RGBA16F));
    geopass->addRenderer(_grid);

    auto overlayPass = std::make_shared<RenderPass>();
    _overlayPass = overlayPass;
    manager->addPass(overlayPass);
    auto overlay = _overlayPass.lock();
    overlay
        ->setDepthOutput(std::make_shared<Renderbuffer>("depth",
                                                        Renderbuffer::DEPTH));
    overlay->addOutput(std::make_shared<Texture2D>("overlay"));
    overlay->setCamera(camera);

    auto deferredPass = std::make_shared<RenderPass>();
    _deferredPass = deferredPass;
    manager->addPass(deferredPass);
    _deferredPass.lock()
        ->addOutput(std::make_shared<Texture2D>("shading_out",
                                                Texture::RGB16F));
    _deferredPass.lock()->setCamera(camera);
    _deferredRenderer = new LightAccumulationPlane();
    _deferredPass.lock()->addRenderer(_deferredRenderer);
    _deferredPass.lock()->setBlendFunc(GL_ONE, GL_ONE);

    auto rsmIndirectPass = std::make_shared<RenderPass>();
    _rsmIndirectPass = rsmIndirectPass;
    manager->addPass(rsmIndirectPass);
    rsmIndirectPass ->addOutput(std::make_shared<Texture2D>("rsm_indirect_out",
                                                Texture::RGB16F));
    rsmIndirectPass->setCamera(camera);
    _rsmIndirectPlane = new RSMIndirectPlane();
    rsmIndirectPass->addRenderer(_rsmIndirectPlane);
    rsmIndirectPass->setBlendFunc(GL_ONE, GL_ONE);

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
    pplane = new PixelPlane();
    pplane->setProvider<FinalOut>();
    finalPass->addRenderer(pplane);

    setupDefaultLights();
    setupGBuffer();
    setupShadowPasses();
}

glm::vec4 DeferredRenderer::getPosition(glm::vec2 pixel) const
{
    std::vector<std::string> values = {"outposition"};
    return _geometryPass.lock()->readPixel(values, pixel)[0];
}

void DeferredRenderer::setProperty(std::string name, Property prop)
{
    Object::setProperty(name, prop);
    
    if (name == "GL:showgrid") {
        bool value = prop.getData<bool>();
        _grid->setVisible(value);
    }
}

void DeferredRenderer::setOverrideOutput(std::string output)
{
    _finalPass.lock()->setCustomTextureNameMapping(output, "final_out");
}

void DeferredRenderer::clearOverrideOutput()
{
    _finalPass.lock()->clearCustomTextureNameMapping();
}

void DeferredRenderer::setCamera(std::shared_ptr<Camera> cam)
{
    _geometryPass.lock()->setCamera(cam);
    _overlayPass.lock()->setCamera(cam);
    _pixelPass.lock()->setCamera(cam);
    _deferredPass.lock()->setCamera(cam);
    _rsmIndirectPass.lock()->setCamera(cam);
    RenderConfigurator::setCamera(cam);
}

void DeferredRenderer::setupGBuffer()
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

void DeferredRenderer::setGeometry(std::shared_ptr<Group> grp)
{
    //clear shadow passes
    for(auto p : _shadowPasses) {
        getManager()->removePass(p.second);
    }
    _shadowNode->clear();
    _shadowPasses.clear();

    auto config = getManager()->getConfig();
    if(config.hasProperty("defaultLighting") &&
       config["defaultLighting"].getData<bool>()) {
        setupDefaultLights();
    }
    else {
        _deferredRenderer->setLights(grp->getLights());
        _rsmIndirectPlane->setLights(grp->getLights());
    }
    setRenderersFromGroup(grp);
    _deferredRenderer->setShadowPasses(_shadowPasses);
    _rsmIndirectPlane->setShadowPasses(_shadowPasses);

    if (grp->hasProperty("RSM:searchRadius")) {
        _rsmIndirectPlane->setSearchRadius(grp->getProperty("RSM:searchRadius").getData<double>());
    }
    if (grp->hasProperty("RSM:intensity")) {
        _rsmIndirectPlane->setIntensity(grp->getProperty("RSM:intensity").getData<double>());
    }
}

void DeferredRenderer::setupShadowPasses()
{
    auto shadowShader = std::make_shared<ShaderProgram>();
    _shadowNode = std::make_shared<ShaderRenderNode>(shadowShader);
    shadowShader
        ->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert", 
                            ShaderProgram::VERTEX);
    shadowShader
        ->addShaderFromFile("../plugins/render/defaultShaders/shadow.frag", 
                            ShaderProgram::FRAGMENT);
}

void DeferredRenderer::addRendererFromObject(std::shared_ptr<GeoObject> obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            _gbufferNode->addRenderer(new PolygonRenderer(obj));
            _shadowNode->addRenderer(new PolygonRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new EdgeRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void DeferredRenderer::createShadowPass(SpotLightPtr spot)
{
    Light::ShadowInfo info = spot->getShadowInfo();
    if(!info._enabled) return;

    auto shadowPass = std::make_shared<RenderPass>();
    size_t shadowCount = _shadowPasses.size();

    _shadowPasses[spot] = shadowPass;

    auto camera = std::make_shared<Camera>();
    camera->setResolution(info._size.x, info._size.y);
    camera->setTransformation(spot->getWorldTransformation());
    camera->setFov(spot->getConeAngle() * 2);
    camera->setNear(info._near);
    camera->setFar(info._far);
    shadowPass->setCamera(camera);
    shadowPass
        ->setDepthOutput(std::make_shared<Texture2D>("shadow",
                                                     Texture::DEPTH32F));
    shadowPass->addOutput(std::make_shared<Texture2D>("shadow_position",
                                                             Texture::RGBA16F));
    shadowPass->addOutput(std::make_shared<Texture2D>("shadow_normal",
                                                             Texture::RGBA16F));
    shadowPass->addOutput(std::make_shared<Texture2D>("shadow_flux"));

    shadowPass->addGeometryShaderNode(_shadowNode);
    shadowPass->setClearDepth(1.);
    static const float PI = 3.14159265359;
    shadowPass->setProperty("coneangle", spot->getConeAngle() * PI /180);

    getManager()->insertPassAfter(_geometryPass, shadowPass);
}

void DeferredRenderer::addRendererFromLight(LightPtr obj)
{
    RenderConfigurator::addRendererFromLight(obj);

    switch(obj->getLightType()) {
        case Light::POINT:
        case Light::DISTANT:
            break;
        case Light::SPOT:
           createShadowPass(std::dynamic_pointer_cast<SpotLight>(obj));
    }
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

void DeferredRenderer::setupDefaultLights()
{
    static const double coneangle = 2 * 3.14159265359;
    auto light1 = std::make_shared<DistantLight>(.8, glm::vec4(1));
    light1->setTransformation(createTransFromZVec(glm::vec3(-1, -1, -1)));

    auto light2 = std::make_shared<DistantLight>(.3, glm::vec4(1));
    light2->setTransformation(createTransFromZVec(glm::vec3(5, 1, -1)));

    auto light3 = std::make_shared<DistantLight>(.1, glm::vec4(1));
    light3->setTransformation(createTransFromZVec(glm::vec3(0, 0, -1)));

    std::vector<LightPtr> lights = {light1, light2, light3};
    _deferredRenderer->setLights(lights);
}

