#ifndef SHADER_RENDER_NODE_H

#define SHADER_RENDER_NODE_H

#include "memory"
#include "vector"
#include "mutex"

#include "../datatypes/Object/object.h"

namespace MindTree
{
namespace GL
{

class ShaderProgram;
class Renderer;
class RenderConfig;

class ShaderRenderNode
{
public:
    ShaderRenderNode(std::shared_ptr<ShaderProgram> program);
    ~ShaderRenderNode();

    void addRenderer(Renderer *renderer);
    void render(CameraPtr camera, glm::ivec2 resolution, const RenderConfig &config);
    std::shared_ptr<ShaderProgram> program();
    const std::vector<std::shared_ptr<Renderer>>& renders();
    void clear();

private:
    friend class RenderPass;
    void init();

    std::shared_ptr<ShaderProgram> _program;
    std::vector<std::shared_ptr<Renderer>> _renders;

    std::atomic<bool> _initialized;
};


}
}
#endif
