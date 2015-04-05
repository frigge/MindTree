#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "unordered_map"
#include "render_block.h"

class SpotLight;
class Light;
namespace MindTree {
namespace GL {

class LightAccumulationPlane;
class RSMIndirectPlane;

class GBufferRenderBlock : public GeometryRenderBlock
{
public:
    GBufferRenderBlock(std::weak_ptr<RenderPass> geopass);
    ~GBufferRenderBlock();
    void init();

    void setProperty(std::string name, Property prop);

protected:
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;

private:
    void setupGBuffer();

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
};

class ShadowMappingRenderBlock;
class SinglePointRenderer;
class DeferredLightingRenderBlock : public RenderBlock
{
public:
    DeferredLightingRenderBlock(ShadowMappingRenderBlock *shadowBlock=nullptr);
    void init();

    void setGeometry(std::shared_ptr<Group> grp);
    void setProperty(std::string name, Property prop);

private:
    void setupDefaultLights();
    LightAccumulationPlane *_deferredRenderer;
    std::weak_ptr<RenderPass> _deferredPass;

    ShadowMappingRenderBlock *_shadowBlock;

    std::vector<std::shared_ptr<Light>> _defaultLights;
    std::vector<std::shared_ptr<Light>> _sceneLights;
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

    SinglePointRenderer *_viewCenter;
};

}
}

#endif
