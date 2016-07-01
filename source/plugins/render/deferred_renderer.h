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

class ShadowMappingRenderBlock;
class SinglePointRenderer;
class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setOverrideOutput(std::string output) override;
    void clearOverrideOutput() override;
    glm::vec4 getPosition(glm::vec2 pixel) const override;

    void setCamera(std::shared_ptr<Camera> cam) override;

    void setProperty(const std::string &name, const Property &prop) override;

private:
    RenderPass *_overlayPass;
    RenderPass *_pixelPass;
    RenderPass *_finalPass;

    SinglePointRenderer *_viewCenter;
};

}
}

#endif
