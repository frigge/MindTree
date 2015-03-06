#define GLM_SWIZZLE
#include "deferred_renderer.h"
#include "light_accumulation_plane.h"

using namespace MindTree;
using namespace GL;

std::weak_ptr<ShaderProgram> LightAccumulationPlane::_defaultProgram;

LightAccumulationPlane::LightAccumulationPlane()
{
}

LightAccumulationPlane::~LightAccumulationPlane()
{
}

std::shared_ptr<ShaderProgram> LightAccumulationPlane::getProgram()
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

void LightAccumulationPlane::setLights(std::vector<std::shared_ptr<Light>> lights)
{
    std::lock_guard<std::mutex> lock(_lightsLock);
    _lights = lights;
}

void LightAccumulationPlane::setShadowPasses(std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> shadowPasses)
{
    std::lock_guard<std::mutex> lock(_shadowPassesLock);
    _shadowPasses = shadowPasses;
}

void LightAccumulationPlane::draw(const CameraPtr /* camera */, 
                                  const RenderConfig& /* config */, 
                                  std::shared_ptr<ShaderProgram> program)
{
    glBlendEquation(GL_FUNC_ADD);
    UniformStateManager states(program);
    std::lock_guard<std::mutex> lock(_lightsLock);
    std::lock_guard<std::mutex> lock2(_shadowPassesLock);
    static const float PI = 3.14159265359;
    for (const LightPtr light : _lights) {
        states.addState("light.shadow", static_cast<int>(light->getShadowInfo()._enabled));
        if(_shadowPasses.find(light) != _shadowPasses.end()) {
            auto shadowmap = _shadowPasses[light].lock()->getOutDepthTexture();
            program->setTexture(shadowmap, "shadow");
            auto shadowcam = _shadowPasses[light].lock()->getCamera();
            glm::mat4 mvp = shadowcam->getProjection() 
                * shadowcam->getViewMatrix();
            states.addState("light.shadowmvp", mvp);
        }
        double coneangle = 360;
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

