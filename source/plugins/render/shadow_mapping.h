#ifndef MT_GL_SHADOW_MAPPING_H
#define MT_GL_SHADOW_MAPPING_H

#include "render_block.h"

class SpotLight;

namespace MindTree {
namespace GL {

class ShaderRenderNode;
class ShadowMappingRenderBlock : public RenderBlock
{
public:
    ShadowMappingRenderBlock();
    void init();

    void setGeometry(std::shared_ptr<Group> grp);
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> getShadowPasses() const;

protected:
    virtual void addRendererFromLight(std::shared_ptr<Light> obj);
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;
    virtual std::weak_ptr<RenderPass> createShadowPass(std::shared_ptr<SpotLight> spot);

private:
    std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> _shadowPasses;
    std::shared_ptr<ShaderRenderNode> _shadowNode;
};

}
}
#endif
