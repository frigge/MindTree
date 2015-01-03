#ifndef RENDER_SETUP_H_RJTLMJKI
#define RENDER_SETUP_H_RJTLMJKI

#include "memory"
#include "vector"

class Widget3DManager;
class QGLContext;

class Group;
class Camera;
class AbstractTransformable;
class GeoObject;
class Light;
class Empty;

namespace MindTree {
namespace GL {

class RenderPass;
class RenderManager;
class ShaderProgram;

class RenderConfigurator
{
public:
    RenderConfigurator(QGLContext *context, std::shared_ptr<Camera> camera);
    void startRendering();
    void stopRendering();

    void setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> getCamera() const;

    RenderManager* getManager();

    virtual void setGeometry(std::shared_ptr<Group> grp);

    void setRenderersFromGroup(std::shared_ptr<Group> group);
    void addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group);
    virtual void addRendererFromTransformable(std::shared_ptr<AbstractTransformable> transformable);
    virtual void addRendererFromObject(std::shared_ptr<GeoObject> obj);
    virtual void addRendererFromLight(std::shared_ptr<Light> obj);
    virtual void addRendererFromCamera(std::shared_ptr<Camera> obj);
    virtual void addRendererFromEmpty(std::shared_ptr<Empty> obj);

protected:
    std::weak_ptr<RenderPass> _geometryPass;

private:
    std::unique_ptr<MindTree::GL::RenderManager> _rendermanager;
    std::shared_ptr<Camera> _camera;
};

class ForwardRenderer : public RenderConfigurator
{
public:
    ForwardRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *manager=nullptr);

    void setGeometry(std::shared_ptr<Group> grp) override;

private:
    void setupDefaultLights();

    uint _maxLightCount;
};

class  DeferredRenderer : public RenderConfigurator
{
public:
    DeferredRenderer(QGLContext *context, std::shared_ptr<Camera> camera, Widget3DManager *widgetManager);

    void setGeometry(std::shared_ptr<Group> grp);

private:
    void setupDefaultLights();

    std::shared_ptr<ShaderProgram> _gbufferShader;
};

}
}

#endif /* end of include guard: RENDER_SETUP_H_RJTLMJKI */
