#ifndef RENDER_BLOCK_H
#define RENDER_BLOCK_H

#include "memory"
#include "vector"

#include "data/mtobject.h"

class Camera;
class AbstractTransformable;
class GeoObject;
class Light;
class Empty;
class Group;

namespace MindTree {
class Benchmark;
namespace GL{

class RenderPass;
class RenderTree;
class RenderConfigurator;

class Texture2D;
class RenderBlock : public Object
{
public:
    RenderBlock();
    virtual void init() = 0;
    void setBenchmark(std::shared_ptr<Benchmark> benchmark);
    std::weak_ptr<Benchmark> getBenchmark() const;

    virtual void setCamera(std::shared_ptr<Camera> camera);
    std::weak_ptr<Camera> getCamera() const;

    virtual void setGeometry(std::shared_ptr<Group> grp);
    std::shared_ptr<RenderPass> addPass();
    void setEnabled(bool enable);
    std::vector<Texture2D*> getOutputs() const;
    void setProperty(std::string name, Property prop);

protected:
    virtual void setRenderersFromGroup(std::shared_ptr<Group> group);
    void addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group);
    virtual void addRendererFromTransformable(std::shared_ptr<AbstractTransformable> transformable);
    virtual void addRendererFromObject(std::shared_ptr<GeoObject> obj);
    virtual void addRendererFromLight(std::shared_ptr<Light> obj);
    virtual void addRendererFromCamera(std::shared_ptr<Camera> obj);
    virtual void addRendererFromEmpty(std::shared_ptr<Empty> obj);
    void addOutput(Texture2D *output);

    RenderConfigurator *_config;
    std::shared_ptr<Benchmark> _benchmark;

private:
    friend class RenderConfigurator;
    std::vector<std::weak_ptr<RenderPass>> _passes;
    std::weak_ptr<Camera> _camera;
    std::vector<Texture2D*> _outputs;
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
