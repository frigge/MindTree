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
class GridRenderer;

class RenderConfigurator : public Object
{
public:
    RenderConfigurator(QGLContext *context, std::shared_ptr<Camera> camera);
    void startRendering();
    void stopRendering();

    virtual void setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> getCamera() const;

    RenderTree* getManager();

    void setGeometry(std::shared_ptr<Group> grp);

    virtual void setOverrideOutput(std::string output);
    virtual void clearOverrideOutput();
    virtual glm::vec4 getPosition(glm::vec2 pixel) const;

    void addRenderBlock(std::shared_ptr<RenderBlock> block);

    void setProperty(std::string name, Property prop);

    std::weak_ptr<RenderPass> getGeometryPass() const;

    int getPolygonCount() const;
    int getVertexCount() const;

protected:
    std::weak_ptr<RenderPass> _geometryPass;
    GridRenderer* _grid;

private:
    int _vertexCount;
    int _polyCount;
    std::shared_ptr<RenderTree> _rendertree;
    std::vector<std::shared_ptr<RenderBlock>> _renderBlocks;
    std::shared_ptr<Camera> _camera;
};

}
}

#endif /* end of include guard: RENDER_SETUP_H_RJTLMJKI */
