#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "render_setup.h"
#include "primitive_renderer.h"

namespace MindTree {
namespace GL {

class DeferredRendererPass : public FullscreenQuadRenderer
{
public:
    DeferredRendererPass();
    virtual ~DeferredRendererPass();

    std::shared_ptr<ShaderProgram> getProgram();

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    static std::weak_ptr<ShaderProgram> _defaultProgram;
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

    std::weak_ptr<ShaderRenderNode> _gbufferNode;
    DeferredRendererPass *_deferredRenderer;
};

}
}

#endif
