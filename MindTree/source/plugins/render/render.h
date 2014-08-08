#ifndef RENDER_GR953KUN

#define RENDER_GR953KUN

#include "memory"
#include "mutex"
#include "vector"
#include "../datatypes/Object/object.h"
#include "glm/glm.hpp"

class GeoObject;
namespace MindTree
{
namespace GL
{

class VAO;
class FBO;

class ShaderProgram;

class RenderConfig;

class Renderer
{
public:
    Renderer();
    virtual ~Renderer();

    void render(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

    void setTransformation(glm::mat4 trans);
    glm::mat4 getTransformation();
    glm::mat4 getGlobalTransformation();

    void setParent(Renderer *parent);
    Renderer* getParent();
    void addChild(Renderer *child);

    void setVisible(bool visible);

    virtual std::shared_ptr<ShaderProgram> getProgram() = 0;

protected:
    friend class ShaderRenderNode;

    virtual void init() = 0;
    virtual void initVAO() = 0;
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program) = 0;

    std::shared_ptr<VAO> _vao;
    std::vector<std::unique_ptr<Renderer>> _children;

private:
    void _init();

    bool _initialized, _visible;
    glm::mat4 _transformation;

    Renderer* _parent;
};

class ShaderRenderNode
{
public:
    ShaderRenderNode(std::shared_ptr<ShaderProgram> program);
    ~ShaderRenderNode();

    void addRenderer(Renderer *renderer);
    void render(CameraPtr camera, glm::ivec2 resolution, const RenderConfig &config);
    std::shared_ptr<ShaderProgram> program();
    std::vector<std::shared_ptr<Renderer>> renders();
    void clear();

private:
    friend class RenderPass;
    void init();

    std::shared_ptr<ShaderProgram> _program;
    std::vector<std::shared_ptr<Renderer>> _renders;
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: RENDER_GR953KUN */
