#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "primitive_renderer.h"

class SpotLight;
namespace MindTree {
namespace GL {

class LightAccumulationPass : public FullscreenQuadRenderer
{
public:
    LightAccumulationPass();
    virtual ~LightAccumulationPass();

    std::shared_ptr<ShaderProgram> getProgram();

    void setLights(std::vector<std::shared_ptr<Light>> lights);
    void setShadowPasses(std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> shadowPasses);

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    static std::weak_ptr<ShaderProgram> _defaultProgram;
    std::mutex _shadowPassesLock;
    std::mutex _lightsLock;
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;
    std::vector<std::shared_ptr<Light>> _lights;
};

class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setGeometry(std::shared_ptr<Group> grp);

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
    LightAccumulationPass *_deferredRenderer;
    std::weak_ptr<RenderPass> _deferredPass;
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;
};

}
}

#endif
