#ifndef LIGHT_ACCUMULATION_PLANE_H
#define LIGHT_ACCUMULATION_PLANE_H

#include "pixel_plane.h"
#include "renderpass.h"

class SpotLight;
namespace MindTree {
namespace GL {

class LightAccumulationPlane : public PixelPlane
{
public:
    LightAccumulationPlane();
    virtual ~LightAccumulationPlane();

    void setLights(std::vector<std::shared_ptr<Light>> lights);
    void setShadowPasses(std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> shadowPasses);

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void drawLight(const LightPtr light, std::shared_ptr<ShaderProgram> program);

    std::vector<LightPtr> getLight() const;
    std::unordered_map<LightPtr, std::weak_ptr<RenderPass>> getShadowPases() const;

private:
    mutable std::mutex _shadowPassesLock;
    mutable std::mutex _lightsLock;
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;
    std::vector<std::shared_ptr<Light>> _lights;
};

}
}
#endif
