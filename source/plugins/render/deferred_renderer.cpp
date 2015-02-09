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
    _lights = lights;
}

void LightAccumulationPass::draw(const CameraPtr /* camera */, 
                                  const RenderConfig& /* config */, 
                                  std::shared_ptr<ShaderProgram> program)
{
    glBlendEquation(GL_FUNC_ADD);
    UniformStateManager states(program);
    for (const LightPtr light : _lights) {
        double coneangle = 2 * 3.14159265359;
        if(light->getLightType() == Light::SPOT)
            coneangle = std::static_pointer_cast<SpotLight>(light)->getConeAngle();

        states.addState("light.pos", 
                        glm::vec4(light->getPosition(), 
                                               light->getLightType() == Light::DISTANT ? 0 : 1));
        states.addState("light.color", light->getColor());
        states.addState("light.intensity", light->getIntensity());
        states.addState("light.coneangle", coneangle);
        states.addState("light.directional", light->getLightType() == Light::DISTANT);
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

    _geometryPass = manager->addPass<GL::RenderPass>();
    _geometryPass.lock()->setCamera(camera);
    _geometryPass.lock()->setEnableBlending(false);

    _geometryPass.lock()->setDepthOutput(std::make_shared<GL::Texture2D>("depth", GL::Texture::DEPTH));
    _geometryPass.lock()->addOutput(std::make_shared<GL::Texture2D>("outcolor"));
    _geometryPass.lock()->addOutput(std::make_shared<GL::Texture2D>("outnormal", GL::Texture::RGBA16F));
    _geometryPass.lock()->addOutput(std::make_shared<GL::Texture2D>("outposition", GL::Texture::RGBA16F));
    _geometryPass.lock()->addRenderer(grid);

    _geometryPass.lock()->setBlendFunc(GL_ONE, GL_ONE);

    auto overlaypass = manager->addPass<GL::RenderPass>();
    overlaypass->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
    overlaypass->addOutput(std::make_shared<GL::Texture2D>("overlay"));
    overlaypass->setCamera(camera);

    auto deferredPass = manager->addPass<GL::RenderPass>();
    deferredPass->addOutput(std::make_shared<GL::Texture2D>("shading_out"));
    deferredPass->setCamera(camera);
    _deferredRenderer = new LightAccumulationPass();
    deferredPass->addRenderer(_deferredRenderer);
    deferredPass->setBlendFunc(GL_ONE, GL_ONE);

    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(overlaypass);

    auto *pixelPass = manager->addPass<GL::RenderPass>().get();
    pixelPass->setCamera(camera);
    pixelPass->addRenderer(new GL::FullscreenQuadRenderer());

    setupDefaultLights();
    setupGBuffer();
}

void DeferredRenderer::setupGBuffer()
{
    std::shared_ptr<ShaderProgram> gbufferShader = std::make_shared<ShaderProgram>();
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
    auto config = getManager()->getConfig();
    if(config.hasProperty("defaultLighting") &&
       config["defaultLighting"].getData<bool>()) {
        setupDefaultLights();
    }
    else {
        _deferredRenderer->setLights(grp->getLights());
    }
    setRenderersFromGroup(grp);
}

void DeferredRenderer::addRendererFromObject(std::shared_ptr<GeoObject> obj)
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

void DeferredRenderer::setupDefaultLights()
{
    static const double coneangle = 2 * 3.14159265359;
    auto light1 = std::make_shared<DistantLight>(.8, glm::vec4(1));
    light1->setPosition(-1, -1, -1);

    auto light2 = std::make_shared<DistantLight>(.3, glm::vec4(1));
    light2->setPosition(5, 1, -1);

    auto light3 = std::make_shared<DistantLight>(.1, glm::vec4(1));
    light3->setPosition(0, 0, -1);

    std::vector<LightPtr> lights = {light1, light2, light3};
    _deferredRenderer->setLights(lights);
}

