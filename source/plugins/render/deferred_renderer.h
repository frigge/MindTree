#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "unordered_map"

class SpotLight;
namespace MindTree {
namespace GL {

class GridRenderer;
class LightAccumulationPlane;
class RSMIndirectPlane;

class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setGeometry(std::shared_ptr<Group> grp);
    void setCamera(std::shared_ptr<Camera> cam);

    void setOverrideOutput(std::string output) override;
    void clearOverrideOutput() override;

    void setProperty(std::string name, Property prop) override;

protected:
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;
    void addRendererFromLight(std::shared_ptr<Light> obj) override;

private:
    std::weak_ptr<RenderPass> setupShadowPass();
    void setupDefaultLights();
    void setupGBuffer();
    void setupShadowPasses();
    void createShadowPass(std::shared_ptr<SpotLight> spot);

    std::vector<std::shared_ptr<RenderBlock>> _renderBlocks;

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
    std::shared_ptr<ShaderRenderNode> _shadowNode;
    LightAccumulationPlane *_deferredRenderer;
    RSMIndirectPlane *_rsmIndirectPlane;
    std::weak_ptr<RenderPass> _deferredPass;
    std::weak_ptr<RenderPass> _pixelPass;
    std::weak_ptr<RenderPass> _overlayPass;
    std::weak_ptr<RenderPass> _finalPass;
    std::weak_ptr<RenderPass> _rsmIndirectPass;
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;

    GridRenderer* _grid;
};

}
}

#endif
