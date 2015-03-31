#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "unordered_map"
#include "render_block.h"

class SpotLight;
namespace MindTree {
namespace GL {

class LightAccumulationPlane;
class RSMIndirectPlane;

class DeferredRenderBlock : public GeometryRenderBlock
{
public:
    DeferredRenderBlock(std::weak_ptr<RenderPass> geopass);
    ~DeferredRenderBlock();
    void init();

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

    RSMIndirectPlane *_rsmIndirectHighResPlane;
    RSMIndirectPlane *_rsmIndirectLowResPlane;

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
    std::shared_ptr<ShaderRenderNode> _shadowNode;
    LightAccumulationPlane *_deferredRenderer;
    std::weak_ptr<RenderPass> _deferredPass;

    std::weak_ptr<RenderPass> _rsmIndirectPass;
    std::weak_ptr<RenderPass> _rsmIndirectLowResPass;
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;
};

class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setOverrideOutput(std::string output) override;
    void clearOverrideOutput() override;
    glm::vec4 getPosition(glm::vec2 pixel) const;

    void setCamera(std::shared_ptr<Camera> cam);

    void setProperty(std::string name, Property prop) override;

private:
    std::weak_ptr<RenderPass> _overlayPass;
    std::weak_ptr<RenderPass> _pixelPass;
    std::weak_ptr<RenderPass> _finalPass;
};

}
}

#endif
