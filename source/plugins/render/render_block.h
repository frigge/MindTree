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

class RenderBlock : public Object
{
public:
    RenderBlock();
    virtual void setGeometry();
    virtual void setCamera(std::shared_ptr<Camera> camera);

protected:
    virtual void addRendererFromTransformable(std::shared_ptr<AbstractTransformable> transformable);
    virtual void addRendererFromObject(std::shared_ptr<GeoObject> obj);
    virtual void addRendererFromLight(std::shared_ptr<Light> obj);
    virtual void addRendererFromCamera(std::shared_ptr<Camera> obj);
    virtual void addRendererFromEmpty(std::shared_ptr<Empty> obj);

private:
    std::vector<std::weak_ptr<RenderPass>> _passes;
};

}
}

#endif
