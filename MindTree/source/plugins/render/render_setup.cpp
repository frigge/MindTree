#include "render.h"
#include "renderpass.h"
#include "primitive_renderer.h"
#include "rendermanager.h"
#include "../3dwidgets/widgets.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "data/debuglog.h"

#include "render_setup.h"

using namespace MindTree;
using namespace GL;

RenderConfigurator::RenderConfigurator(QGLContext *context, CameraPtr camera) :
    _rendermanager(new RenderManager(context)), _camera(camera)
{

}

void RenderConfigurator::startRendering()
{
    MindTree::GL::RenderThread::addManager(_rendermanager.get());
}

void RenderConfigurator::stopRendering()
{
    MindTree::GL::RenderThread::removeManager(_rendermanager.get());
}

RenderManager* RenderConfigurator::getManager()
{
    return _rendermanager.get();
}

void RenderConfigurator::setGeometry(std::shared_ptr<Group> grp)
{
}

void RenderConfigurator::setCamera(std::shared_ptr<Camera> camera)
{
    _camera = camera;
}

std::shared_ptr<Camera> RenderConfigurator::getCamera() const
{
    return _camera;
}

ForwardRenderer::ForwardRenderer(QGLContext *context, CameraPtr camera, Widget3DManager *widgetManager)
    : RenderConfigurator(context, camera), _maxLightCount(5)
{
    RenderManager *manager = getManager();
    auto config = manager->getConfig();
    config.setProperty("defaultLighting", true);
    manager->setConfig(config);

    auto pass = manager->addPass<GL::RenderPass>();
    pass->setCamera(camera);

    auto grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    grid->setTransformation(trans);
    grid->setBorderColor(glm::vec4(.5, .5, .5, .5));
    grid->setAlternatingColor(glm::vec4(.8, .8, .8, .8));
    grid->setBorderWidth(2.);

    pass->addRenderer(grid);
    //pass->setDepthOutput(std::make_shared<GL::Texture2D>("depth", GL::Texture::DEPTH));
    pass->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
    //pass->addOutput(std::make_shared<GL::Texture2D>("outnormal"));
    //pass->addOutput(std::make_shared<GL::Texture2D>("outposition"));

//    auto overlaypass = manager->addPass<GL::RenderPass>();
//    overlaypass->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
//    overlaypass->addOutput(std::make_shared<GL::Texture2D>("overlay"));
//    overlaypass->addOutput(std::make_shared<GL::Renderbuffer>("id"));
//    overlaypass->addOutput(std::make_shared<GL::Renderbuffer>("position", GL::Renderbuffer::RGBA16F));
//    overlaypass->setCamera(activeCamera);
//
    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(pass);

    //auto *pixelPass = manager->addPass<GL::RenderPass>().get();
    //pixelPass->addRenderer(new GL::FullscreenQuadRenderer());

    setupDefaultLights();
}

void ForwardRenderer::setupDefaultLights()
{
    RenderPass *pass = getManager()->getPass(0);

    //light0
    pass->setProperty("light0.intensity", .8);
    pass->setProperty("light0.color", glm::vec4(1));
    pass->setProperty("light0.pos", glm::vec3(50, 50, 50));

    //light1
    pass->setProperty("light1.intensity", .3);
    pass->setProperty("light1.color", glm::vec4(1));
    pass->setProperty("light1.pos", glm::vec3(-50, -10, 10));

    //light2
    pass->setProperty("light2.intensity", .1);
    pass->setProperty("light2.color", glm::vec4(1));
    pass->setProperty("light2.pos", glm::vec3(0, 0, 50));

    //light3
    pass->setProperty("light3.intensity", .0);
    pass->setProperty("light3.color", glm::vec4(1));
    pass->setProperty("light3.pos", glm::vec3(0, 0, 0));

    //light4
    pass->setProperty("light4.intensity", .0);
    pass->setProperty("light4.color", glm::vec4(1));
    pass->setProperty("light4.pos", glm::vec3(0, 0, 0));
}

void ForwardRenderer::setGeometry(std::shared_ptr<Group> grp)
{
    RenderPass *pass = getManager()->getPass(0);
    pass->setRenderersFromGroup(grp);

    auto config = getManager()->getConfig();
    if(config.hasProperty("defaultLighting")) {
        pass->setProperty("defaultLighting", config.getProperty("defaultLighting"));
    }
    else if(config["defaultLighting"].getData<bool>()) {
        setupDefaultLights();
        return;
    }

    std::vector<LightPtr> lights = grp->getLights();
    for(size_t i = 0; i < _maxLightCount; ++i) {
        std::string lightName = "light";
        lightName += std::to_string(i);
        if (i < lights.size()) {
            LightPtr light = lights[i];
            pass->setProperty(lightName + ".intensity", light->getIntensity());
            pass->setProperty(lightName + ".color", light->getColor());
            pass->setProperty(lightName + ".pos", light->getPosition());

            dbout("setting " << lightName << ".intensity: " << light->getIntensity());
            dbout("setting " << lightName << ".color: " << glm::to_string(light->getColor()));
            dbout("setting " << lightName << ".pos: " << glm::to_string(light->getPosition()));
        }
        else {
            pass->setProperty(lightName + ".intensity", 0.0);
            pass->setProperty(lightName + ".color", glm::vec4(0));
            pass->setProperty(lightName + ".pos", glm::vec3(0));
        }
    }
}
