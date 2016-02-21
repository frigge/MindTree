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
    void init() override;

    void setGeometry(std::shared_ptr<Group> grp) override;
    std::unordered_map<std::shared_ptr<Light>, RenderPass*> getShadowPasses() const;

protected:
    virtual void addRendererFromLight(std::shared_ptr<Light> obj) override;
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;
    virtual RenderPass* createShadowPass(std::shared_ptr<SpotLight> spot);

private:
    std::unordered_map<std::shared_ptr<Light>, RenderPass*> _shadowPasses;
    std::shared_ptr<ShaderRenderNode> _shadowNode;
};

}
}
#endif
