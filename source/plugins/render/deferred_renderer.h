#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "unordered_map"

class SpotLight;
namespace MindTree {
namespace GL {

class LightAccumulationPlane;

class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setGeometry(std::shared_ptr<Group> grp);
    void setCamera(std::shared_ptr<Camera> cam);

protected:
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;
    void addRendererFromLight(std::shared_ptr<Light> obj) override;

private:
    std::weak_ptr<RenderPass> setupShadowPass();
    void setupDefaultLights();
    void setupGBuffer();
    void setupShadowPasses();
    void createShadowPass(std::shared_ptr<SpotLight> spot);

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
    std::shared_ptr<ShaderRenderNode> _shadowNode;
    LightAccumulationPlane *_deferredRenderer;
    std::weak_ptr<RenderPass> _deferredPass;
    std::weak_ptr<RenderPass> _pixelPass;
    std::weak_ptr<RenderPass> _overlayPass;
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;
};

}
}

#endif
