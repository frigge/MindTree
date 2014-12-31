#ifndef RENDER_SETUP_H_RJTLMJKI
#define RENDER_SETUP_H_RJTLMJKI

#include "memory"

class Widget3DManager;
class QGLContext;

class Group;
class Camera;

namespace MindTree {
namespace GL {

class RenderManager;

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

};

}
}

#endif /* end of include guard: RENDER_SETUP_H_RJTLMJKI */
