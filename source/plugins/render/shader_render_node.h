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
    ShaderRenderNode(ShaderProgram *program);

    void addRenderer(Renderer *renderer);
    void render(CameraPtr camera, glm::ivec2 resolution, const RenderConfig &config);
    ShaderProgram* program();
    std::vector<Renderer*> renders();
    void setResourceManager(ResourceManager *manager);
    void clear();

private:
    friend class RenderPass;
    void init();

    ResourceManager *_resourceManager;

    ShaderProgram *_program;
    std::vector<std::unique_ptr<Renderer>> _renders;
    std::mutex _rendersLock;

    std::atomic<bool> _initialized;
};


}
}
#endif
