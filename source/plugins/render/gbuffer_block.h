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
    GBufferRenderBlock(std::weak_ptr<RenderPass> geopass);
    ~GBufferRenderBlock();
    void init();

    void setProperty(std::string name, Property prop);

protected:
    void addRendererFromObject(std::shared_ptr<GeoObject> obj) override;

private:
    void setupGBuffer();

    std::shared_ptr<ShaderRenderNode> _gbufferNode;
};
}
}
#endif
