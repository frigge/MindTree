#ifndef MT_GBUFFER_BLOCK_H
#define MT_GBUFFER_BLOCK_H

#include "render_setup.h"
#include "render_block.h"

namespace MindTree
{
namespace GL
{
class GBufferRenderBlock : public GeometryRenderBlock
{
public:
    GBufferRenderBlock(RenderPass *geopass);
    ~GBufferRenderBlock();
    void init() override;

    void setProperty(const std::string &name, const Property &prop) override;

protected:
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;

private:
    void setupGBuffer();

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
};
}
}
#endif
