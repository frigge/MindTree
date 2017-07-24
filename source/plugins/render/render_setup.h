#ifndef RENDER_SETUP_H_RJTLMJKI
#define RENDER_SETUP_H_RJTLMJKI

#include "memory"
#include "vector"

#include "data/mtobject.h"
#include "qtcontext.h"
#include "glwrapper.h"

class Widget3DManager;
class QOpenGLContext;

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
class GridRenderer;
class QtContext;

class RenderConfigurator : public Object
{
public:
    RenderConfigurator(std::shared_ptr<Camera> camera);
    ~RenderConfigurator();

    void startRendering(QtContext &ctx);
    void stopRendering();

    virtual void setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> getCamera() const;

    RenderTree* getTree();

    void setGeometry(std::shared_ptr<Group> grp);

    virtual void setOverrideOutput(std::string output);
    virtual void clearOverrideOutput();
    virtual glm::vec4 getPosition(glm::vec2 pixel) const;

    void addRenderBlock(std::unique_ptr<RenderBlock> &&block);

    void setProperty(const std::string &name, Property prop) override;

    RenderPass* getGeometryPass() const;

    int getPolygonCount() const;
    int getVertexCount() const;

    void addSettings(std::string name, Property prop);
    PropertyMap getSettings() const;

protected:
    RenderPass* _geometryPass;
    GridRenderer* _grid;

private:
    int _vertexCount;
    int _polyCount;
    std::unique_ptr<RenderTree> _rendertree;
    std::vector<std::unique_ptr<RenderBlock>> _renderBlocks;
    std::shared_ptr<Camera> _camera;
    PropertyMap _settings;
};

}
}

#endif /* end of include guard: RENDER_SETUP_H_RJTLMJKI */
