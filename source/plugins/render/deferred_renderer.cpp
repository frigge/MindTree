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
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    MTGLERROR;
}


DeferredRenderer::DeferredRenderer(QGLContext *context, CameraPtr camera, Widget3DManager *widgetManager) :
    RenderConfigurator(context, camera)
{
    RenderManager *manager = getManager();
    auto config = manager->getConfig();
    config.setProperty("defaultLighting", true);
    manager->setConfig(config);

    auto gridpass = manager->addPass<GL::RenderPass>();
    gridpass->setCamera(camera);
    gridpass->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
    gridpass->addOutput(std::make_shared<GL::Texture2D>("grid"));

    auto grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    grid->setTransformation(trans);
    grid->setBorderColor(glm::vec4(.5, .5, .5, .5));
    grid->setAlternatingColor(glm::vec4(.8, .8, .8, .8));
    grid->setBorderWidth(2.);

    gridpass->addRenderer(grid);

    _geometryPass = manager->addPass<GL::RenderPass>();
    _geometryPass.lock()->setCamera(camera);

    _geometryPass.lock()->setDepthOutput(std::make_shared<GL::Texture2D>("depth", GL::Texture::DEPTH));
    _geometryPass.lock()->addOutput(std::make_shared<GL::Texture2D>("outnormal"));
    _geometryPass.lock()->addOutput(std::make_shared<GL::Texture2D>("outposition"));

    auto deferredPass = manager->addPass<GL::RenderPass>();
    deferredPass->addOutput(std::make_shared<GL::Texture2D>("shading_out"));
    deferredPass->setCamera(camera);
    _deferredRenderer = new LightAccumulationPass();
    deferredPass->addRenderer(_deferredRenderer);

    auto overlaypass = manager->addPass<GL::RenderPass>();
    overlaypass->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
    overlaypass->addOutput(std::make_shared<GL::Texture2D>("overlay"));
    overlaypass->setCamera(camera);

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
        std::vector<LightPtr> lights = grp->getLights();
        size_t i = 0;
        for(auto &light : lights) {
            std::string lightName = "light";
            lightName += std::to_string(i);
            _geometryPass.lock()->setProperty(lightName + ".intensity", light->getIntensity());
            _geometryPass.lock()->setProperty(lightName + ".color", light->getColor());
            glm::vec4 pos;
            float coneangle = 2 * 3.14159265359;
            bool directional = false;
            switch(light->getLightType()) {
                case Light::POINT:
                    pos = glm::vec4(light->getPosition(), 1.);
                    break;
                case Light::SPOT:
                    pos = glm::vec4(light->getPosition(), 1.);
                    coneangle = std::static_pointer_cast<SpotLight>(light)->getConeAngle();
                    break;
                case Light::DISTANT:
                    pos = glm::vec4(light->getPosition(), 0.);
                    directional = true;
                    break;
            }
            _geometryPass.lock()->setProperty(lightName + ".pos", pos);
            _geometryPass.lock()->setProperty(lightName + ".coneangle", coneangle);
            _geometryPass.lock()->setProperty(lightName + ".directional", directional);
        }
    }
    setRenderersFromGroup(grp);
}

void DeferredRenderer::addRendererFromObject(std::shared_ptr<GeoObject> obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            _gbufferNode.lock()->addRenderer(new PolygonRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new EdgeRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void DeferredRenderer::setupDefaultLights()
{
    static const double coneangle = 2 * 3.14159265359;
    //light0
    _geometryPass.lock()->setProperty("light0.intensity", .8);
    _geometryPass.lock()->setProperty("light0.color", glm::vec4(1));
    _geometryPass.lock()->setProperty("light0.pos", glm::vec4(-50, -50, -50, 0));
    _geometryPass.lock()->setProperty("light0.coneangle", coneangle);
    _geometryPass.lock()->setProperty("light0.directional", true);

    //light1
    _geometryPass.lock()->setProperty("light1.intensity", .3);
    _geometryPass.lock()->setProperty("light1.color", glm::vec4(1));
    _geometryPass.lock()->setProperty("light1.pos", glm::vec4(50, 10, -10, 0));
    _geometryPass.lock()->setProperty("light1.coneangle", coneangle);
    _geometryPass.lock()->setProperty("light1.directional", true);

    //light2
    _geometryPass.lock()->setProperty("light2.intensity", .1);
    _geometryPass.lock()->setProperty("light2.color", glm::vec4(1));
    _geometryPass.lock()->setProperty("light2.pos", glm::vec4(0, 0, -50, 0));
    _geometryPass.lock()->setProperty("light2.coneangle", coneangle);
    _geometryPass.lock()->setProperty("light2.directional", true);
}

