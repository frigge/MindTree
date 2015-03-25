#include "render.h"
#include "renderpass.h"
#include "primitive_renderer.h"
#include "rendertree.h"
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

#include "forward_renderer.h"

using namespace MindTree;
using namespace GL;

ForwardRenderer::ForwardRenderer(QGLContext *context, CameraPtr camera, Widget3DManager *widgetManager)
    : RenderConfigurator(context, camera), _maxLightCount(5)
{
    RenderTree *manager = getManager();
    auto config = manager->getConfig();
    config.setProperty("defaultLighting", true);
    manager->setConfig(config);

    auto geometryPass = std::make_shared<RenderPass>();
    _geometryPass = geometryPass;
    manager->addPass(geometryPass);
    _geometryPass.lock()->setCamera(camera);

    auto grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    grid->setTransformation(trans);
    grid->setBorderColor(glm::vec4(.5, .5, .5, .5));
    grid->setAlternatingColor(glm::vec4(.8, .8, .8, .8));
    grid->setBorderWidth(2.);

    _geometryPass.lock()->addRenderer(grid);
    _geometryPass.lock()->setDepthOutput(std::make_shared<GL::Renderbuffer>("depth", GL::Renderbuffer::DEPTH));
    if(widgetManager) widgetManager->insertWidgetsIntoRenderPass(_geometryPass.lock());

    setupDefaultLights();
}

void ForwardRenderer::setupDefaultLights()
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

    //light3
    _geometryPass.lock()->setProperty("light3.intensity", .0);
    _geometryPass.lock()->setProperty("light3.color", glm::vec4(1));
    _geometryPass.lock()->setProperty("light3.pos", glm::vec4(0, 0, 0, 0));
    _geometryPass.lock()->setProperty("light3.coneangle", coneangle);
    _geometryPass.lock()->setProperty("light3.directional", true);

    //light4
    _geometryPass.lock()->setProperty("light4.intensity", .0);
    _geometryPass.lock()->setProperty("light4.color", glm::vec4(1));
    _geometryPass.lock()->setProperty("light4.pos", glm::vec4(0, 0, 0, 0));
    _geometryPass.lock()->setProperty("light4.coneangle", coneangle);
    _geometryPass.lock()->setProperty("light4.directional", true);
}

void ForwardRenderer::setGeometry(std::shared_ptr<Group> grp)
{
    auto config = getManager()->getConfig();
    if(config.hasProperty("defaultLighting") &&
       config["defaultLighting"].getData<bool>()) {
        setupDefaultLights();
    }
    else {
        std::vector<LightPtr> lights = grp->getLights();
        for(size_t i = 0; i < _maxLightCount; ++i) {
            std::string lightName = "light";
            lightName += std::to_string(i);
            if (i < lights.size()) {
                LightPtr light = lights[i];
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
            else {
                _geometryPass.lock()->setProperty(lightName + ".intensity", 0.0);
                _geometryPass.lock()->setProperty(lightName + ".color", glm::vec4(1));
                _geometryPass.lock()->setProperty(lightName + ".pos", glm::vec4(0, 0, 0, 1));
                _geometryPass.lock()->setProperty(lightName + ".coneangle", 2 * 3.14156);
            }
        }
    }
    setRenderersFromGroup(grp);
}

