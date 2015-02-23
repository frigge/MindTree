#define GLM_SWIZZLE
#include "render.h"
#include "renderpass.h"
#include "primitive_renderer.h"
#include "rendermanager.h"
#include "../3dwidgets/widgets.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "data/debuglog.h"
#include "polygon_renderer.h"
#include "light_renderer.h"
#include "../datatypes/Object/lights.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "shader_render_node.h"

#include "deferred_renderer.h"

using namespace MindTree;
using namespace GL;

std::weak_ptr<ShaderProgram> LightAccumulationPass::_defaultProgram;

LightAccumulationPass::LightAccumulationPass()
{
}

LightAccumulationPass::~LightAccumulationPass()
{
}

std::shared_ptr<ShaderProgram> LightAccumulationPass::getProgram()
{
    std::shared_ptr<ShaderProgram> prog;
    if(_defaultProgram.expired()) {
        prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/deferredshading.frag", 
                                ShaderProgram::FRAGMENT);
        _defaultProgram = prog;
    }

    return _defaultProgram.lock();
}

void LightAccumulationPass::setLights(std::vector<std::shared_ptr<Light>> lights)
{
    std::lock_guard<std::mutex> lock(_lightsLock);
    _lights = lights;
}

void LightAccumulationPass::setShadowPasses(std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> shadowPasses)
{
    std::lock_guard<std::mutex> lock(_shadowPassesLock);
    _shadowPasses = shadowPasses;
}

void LightAccumulationPass::draw(const CameraPtr /* camera */, 
                                  const RenderConfig& /* config */, 
                                  std::shared_ptr<ShaderProgram> program)
{
    glBlendEquation(GL_FUNC_ADD);
    UniformStateManager states(program);
    std::lock_guard<std::mutex> lock(_lightsLock);
    std::lock_guard<std::mutex> lock2(_shadowPassesLock);
    static const float PI = 3.14159265359;
    for (const LightPtr light : _lights) {
        if(_shadowPasses.find(light) != _shadowPasses.end()) {
            auto shadowmap = _shadowPasses[light].lock()->getOutputTextures()[0];
            program->setTexture(shadowmap, "shadow");
            auto shadowcam = _shadowPasses[light].lock()->getCamera();
            glm::mat4 mvp = shadowcam->getProjection() 
                * shadowcam->getViewMatrix();
            states.addState("light.shadowmvp", mvp);
        }
        double coneangle = 2 * PI;
        if(light->getLightType() == Light::SPOT)
            coneangle = std::static_pointer_cast<SpotLight>(light)->getConeAngle();

        states.addState("light.pos",
                        glm::vec4(light->getPosition(),
                                  light->getLightType() == Light::DISTANT ? 0 : 1));

        glm::vec3 dir(0);
        if (light->getLightType() == Light::DISTANT
            || light->getLightType() == Light::SPOT){
            dir = light->getTransformation()[2].xyz();
        }

        states.addState("light.dir", dir);
        states.addState("light.color", light->getColor());
        states.addState("light.intensity", light->getIntensity());
        states.addState("light.coneangle", coneangle * PI /180);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        MTGLERROR;
    }
}

DeferredRenderer::DeferredRenderer(QGLContext *context, CameraPtr camera, Widget3DManager *widgetManager) :
    RenderConfigurator(context, camera)
{
    RenderManager *manager = getManager();
    auto config = manager->getConfig();
    config.setProperty("defaultLighting", true);
    manager->setConfig(config);

    auto grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    grid->setTransformation(trans);
    grid->setBorderColor(glm::vec4(.3, .3, .3, 1.0));
    grid->setAlternatingColor(glm::vec4(.7, .7, .7, 1.0));
    grid->setBorderWidth(2.);

    _geometryPass = manager->addPass();
    auto geopass = _geometryPass.lock();

    geopass->setCamera(camera);
    geopass->setEnableBlending(false);

    geopass->setDepthOutput(std::make_shared<Texture2D>("depth", 
                                                        Texture::DEPTH));
    geopass->addOutput(std::make_shared<Texture2D>("outcolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outnormal", 
                                                   Texture::RGBA16F));
    geopass->addOutput(std::make_shared<Texture2D>("outposition", 
                                                   Texture::RGBA16F));
    geopass->addRenderer(grid);

    geopass->setBlendFunc(GL_ONE, GL_ONE);

    auto overlay = manager->addPass().lock();
    overlay
        ->setDepthOutput(std::make_shared<Renderbuffer>("depth",
                                                        Renderbuffer::DEPTH));
    overlay->addOutput(std::make_shared<Texture2D>("overlay"));
    overlay->setCamera(camera);

    _deferredPass = manager->addPass();
    _deferredPass.lock()
        ->addOutput(std::make_shared<Texture2D>("shading_out",
                                                Texture::RGB));

    _deferredPass.lock()->setCamera(camera);
    _deferredRenderer = new LightAccumulationPass();
    _deferredPass.lock()->addRenderer(_deferredRenderer);

    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(overlay);

    auto pixelPass = manager->addPass().lock();
    pixelPass->setCamera(camera);
    pixelPass->addRenderer(new FullscreenQuadRenderer());

    setupDefaultLights();
    setupGBuffer();
    setupShadowPasses();
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
    _shadowPasses.clear();

    auto config = getManager()->getConfig();
    if(config.hasProperty("defaultLighting") &&
       config["defaultLighting"].getData<bool>()) {
        setupDefaultLights();
    }
    else {
        _deferredRenderer->setLights(grp->getLights());
        _deferredRenderer->setShadowPasses(_shadowPasses);
    }
    setRenderersFromGroup(grp);
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

    auto shadowPass = getManager()->insertPassAfter(_geometryPass);
    size_t shadowCount = _shadowPasses.size();
    _shadowPasses.insert({spot, shadowPass});

    auto camera = std::make_shared<Camera>();
    camera->setResolution(info._size.x, info._size.y);
    camera->setTransformation(spot->getTransformation());
    shadowPass.lock()->setCamera(camera);
    shadowPass.lock()
        ->setDepthOutput(std::make_shared<Texture2D>("shadow",
                                                     Texture::DEPTH16));

    shadowPass.lock()->addGeometryShaderNode(_shadowNode);
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

