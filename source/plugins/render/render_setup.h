#ifndef RENDER_SETUP_H_RJTLMJKI
#define RENDER_SETUP_H_RJTLMJKI

#include "memory"
#include "vector"

#include "data/mtobject.h"

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
class RenderTree;
class ShaderProgram;
class ShaderRenderNode;
class RenderBlock;

class RenderConfigurator : public Object
{
public:
    RenderConfigurator(QGLContext *context, std::shared_ptr<Camera> camera);
    void startRendering();
    void stopRendering();

    virtual void setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> getCamera() const;

    RenderTree* getManager();

    virtual void setGeometry(std::shared_ptr<Group> grp);

    void setRenderersFromGroup(std::shared_ptr<Group> group);
    void addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group);

    virtual void setOverrideOutput(std::string output);
    virtual void clearOverrideOutput();
    virtual glm::vec4 getPosition(glm::vec2 pixel) const;

    void addRenderBlock(std::shared_ptr<RenderBlock> block);

protected:
    virtual void addRendererFromTransformable(std::shared_ptr<AbstractTransformable> transformable);
    virtual void addRendererFromObject(std::shared_ptr<GeoObject> obj);
    virtual void addRendererFromLight(std::shared_ptr<Light> obj);
    virtual void addRendererFromCamera(std::shared_ptr<Camera> obj);
    virtual void addRendererFromEmpty(std::shared_ptr<Empty> obj);

    std::weak_ptr<RenderPass> _geometryPass;

private:
    std::unique_ptr<RenderTree> _rendertree;
    std::vector<std::shared_ptr<RenderBlock>> _renderBlocks;
    std::shared_ptr<Camera> _camera;
};

}
}

#endif /* end of include guard: RENDER_SETUP_H_RJTLMJKI */
