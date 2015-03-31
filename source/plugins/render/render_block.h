#ifndef RENDER_BLOCK_H
#define RENDER_BLOCK_H

#include "memory"
#include "vector"

class Camera;
class AbstractTransformable;
class GeoObject;
class Light;
class Empty;

namespace MindTree {
namespace GL{

class RenderPass;
class RenderTree;
class RenderConfigurator;

class RenderBlock : public Object
{
public:
    RenderBlock();
    virtual void init() = 0;
    virtual void setCamera(std::shared_ptr<Camera> camera);
    virtual void setGeometry(std::shared_ptr<Group> grp);
    std::shared_ptr<RenderPass> addPass();

protected:
    RenderConfigurator* _config;

private:
    friend class RenderConfigurator;
    std::vector<std::weak_ptr<RenderPass>> _passes;
};

class GeometryRenderBlock : public RenderBlock
{
public:
    GeometryRenderBlock(std::weak_ptr<RenderPass> geopass);
    virtual void setGeometry(std::shared_ptr<Group> grp);

    std::weak_ptr<RenderPass> getGeometryPass() const;

protected:
    virtual void setRenderersFromGroup(std::shared_ptr<Group> group);
    void addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group);
    virtual void addRendererFromTransformable(std::shared_ptr<AbstractTransformable> transformable);
    virtual void addRendererFromObject(std::shared_ptr<GeoObject> obj);
    virtual void addRendererFromLight(std::shared_ptr<Light> obj);
    virtual void addRendererFromCamera(std::shared_ptr<Camera> obj);
    virtual void addRendererFromEmpty(std::shared_ptr<Empty> obj);

    std::weak_ptr<RenderPass> _geometryPass;

private:
    std::weak_ptr<RenderTree> _rendertree;
};

}
}

#endif
