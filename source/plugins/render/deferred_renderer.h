#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "primitive_renderer.h"

namespace MindTree {
namespace GL {

class LightAccumulationPass : public FullscreenQuadRenderer
{
public:
    LightAccumulationPass();
    virtual ~LightAccumulationPass();

    std::shared_ptr<ShaderProgram> getProgram();

    void setLights(std::vector<std::shared_ptr<Light>> lights);

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    static std::weak_ptr<ShaderProgram> _defaultProgram;
    std::vector<std::shared_ptr<Light>> _lights;
};

class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setGeometry(std::shared_ptr<Group> grp);

protected:
    void addRendererFromObject(std::shared_ptr<GeoObject> obj);

private:
    void setupDefaultLights();
    void setupGBuffer();

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
    LightAccumulationPass *_deferredRenderer;
};

}
}

#endif
