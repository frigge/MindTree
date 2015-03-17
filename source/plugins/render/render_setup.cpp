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

#include "render_setup.h"

using namespace MindTree;
using namespace GL;

RenderConfigurator::RenderConfigurator(QGLContext *context, CameraPtr camera) :
    _rendertree(new RenderTree(context)),
    _camera(camera)
{

}

void RenderConfigurator::startRendering()
{
    MindTree::GL::RenderThread::addManager(_rendertree.get());
}

void RenderConfigurator::stopRendering()
{
    MindTree::GL::RenderThread::removeManager(_rendertree.get());
}

glm::vec4 RenderConfigurator::getPosition(glm::vec2 pixel) const
{
    return glm::vec4(0);
}

RenderTree* RenderConfigurator::getManager()
{
    return _rendertree.get();
}

void RenderConfigurator::setOverrideOutput(std::string output)
{
}

void RenderConfigurator::clearOverrideOutput()
{
}

void RenderConfigurator::setRenderersFromGroup(std::shared_ptr<Group> group)
{
    _geometryPass.lock()->clearRenderers();
    addRenderersFromGroup(group->getMembers());
    _geometryPass.lock()->clearUnusedShaderNodes();
}

void RenderConfigurator::addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group)
{
    for(const auto &transformable : group) {
        addRendererFromTransformable(transformable);
    }
}

void RenderConfigurator::addRendererFromTransformable(AbstractTransformablePtr transformable)
{
    assert(transformable);
    switch(transformable->getType()) {
        case AbstractTransformable::GEO:
            addRendererFromObject(std::dynamic_pointer_cast<GeoObject>(transformable));
            break;
        case AbstractTransformable::LIGHT:
            addRendererFromLight(std::dynamic_pointer_cast<Light>(transformable));
            break;
        case AbstractTransformable::CAMERA:
            addRendererFromCamera(std::dynamic_pointer_cast<Camera>(transformable));
            break;
        case AbstractTransformable::EMPTY:
            addRendererFromEmpty(std::dynamic_pointer_cast<Empty>(transformable));
            break;
    }
    addRenderersFromGroup(transformable->getChildren());
}

void RenderConfigurator::addRendererFromObject(GeoObjectPtr obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            _geometryPass.lock()->addGeometryRenderer(new PolygonRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new EdgeRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void RenderConfigurator::addRendererFromLight(LightPtr obj)
{
    assert(obj);
    switch(obj->getLightType()) {
        case Light::POINT:
            _geometryPass.lock()->addGeometryRenderer(new PointLightRenderer(std::dynamic_pointer_cast<PointLight>(obj)));
            break;
        case Light::SPOT:
            _geometryPass.lock()->addGeometryRenderer(new SpotLightRenderer(std::dynamic_pointer_cast<SpotLight>(obj)));
            break;
        case Light::DISTANT:
            _geometryPass.lock()->addGeometryRenderer(new DistantLightRenderer(std::dynamic_pointer_cast<DistantLight>(obj)));
            break;
    }

}

void RenderConfigurator::addRendererFromCamera(CameraPtr obj)
{
    _geometryPass.lock()->addGeometryRenderer(new CameraRenderer(obj));
}

void RenderConfigurator::addRendererFromEmpty(EmptyPtr obj)
{
    _geometryPass.lock()->addGeometryRenderer(new EmptyRenderer(obj));
}

void RenderConfigurator::setGeometry(std::shared_ptr<Group> grp)
{
    setRenderersFromGroup(grp);
}

void RenderConfigurator::setCamera(std::shared_ptr<Camera> camera)
{
    _camera = camera;
    _rendertree->setDirty();
}

std::shared_ptr<Camera> RenderConfigurator::getCamera() const
{
    return _camera;
}

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

