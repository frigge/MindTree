#ifndef DEFERRED_LIGHT_RENDER_BLOCK_H
#define DEFERRED_LIGHT_RENDER_BLOCK_H

#include "render_block.h"

namespace MindTree
{
namespace GL
{
class LightAccumulationPlane;

class DeferredLightingRenderBlock : public RenderBlock
{
public:
    DeferredLightingRenderBlock(ShadowMappingRenderBlock *shadowBlock=nullptr);
    void init();

    void setGeometry(std::shared_ptr<Group> grp);
    void setProperty(std::string name, Property prop);

private:
    void setupDefaultLights();
    LightAccumulationPlane *_deferredRenderer;
    std::weak_ptr<RenderPass> _deferredPass;

    ShadowMappingRenderBlock *_shadowBlock;

    std::vector<std::shared_ptr<Light>> _defaultLights;
    std::vector<std::shared_ptr<Light>> _sceneLights;
};

}
}
#endif
