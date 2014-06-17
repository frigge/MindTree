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

    void setVisible(bool visible);

    virtual std::shared_ptr<ShaderProgram> getProgram() = 0;

protected:
    virtual void init() = 0;
    virtual void initVAO() = 0;
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program) = 0;

    std::shared_ptr<VAO> _vao;

private:
    void _init();

    bool _initialized, _visible;
    glm::mat4 _transformation;
};

class ShaderRenderNode;

class Texture2D;
class RenderPass
{
public:
    RenderPass();
    virtual ~RenderPass();
    void render(const RenderConfig &config);

    void setCamera(CameraPtr camera);
    void setRenderersFromGroup(std::shared_ptr<Group> group);
    void addRendererFromObject(std::shared_ptr<GeoObject> obj);
    void addRenderer(Renderer *renderer);

    void setTarget(std::shared_ptr<FBO> target);
    std::shared_ptr<FBO> getTarget();

    std::vector<std::string> getOutputs();
    void addOutput(std::string out);

    std::vector<std::shared_ptr<Texture2D>> getOutputTextures();
    std::shared_ptr<Texture2D> getOutDepthTexture();

    std::vector<std::shared_ptr<ShaderRenderNode>> getShaderNodes();

    CameraPtr getCamera();

    void setSize(int width, int height);
    glm::ivec2 getResolution();

private:
    void addGeometryRenderer(Renderer *renderer);

    friend class RenderManager;
    void init();

    int _width, _height;
    bool _viewportChanged, _initialized;
    std::shared_ptr<Camera> _camera;
    std::shared_ptr<FBO> _target;
    std::mutex _sizeLock;
    std::mutex _geometryLock;

    std::vector<std::string> _outputs;
    std::vector<std::shared_ptr<ShaderRenderNode>> _shadernodes;
    std::vector<std::shared_ptr<ShaderRenderNode>> _geometryShaderNodes;
    std::vector<std::shared_ptr<Texture2D>> _outputTextures;
    std::shared_ptr<Texture2D> _depthTexture;
};

class ShaderRenderNode
{
public:
    ShaderRenderNode(std::shared_ptr<ShaderProgram> program);
    ~ShaderRenderNode();

    void addRenderer(Renderer *renderer);
    void render(CameraPtr camera, const RenderConfig &config);
    std::shared_ptr<ShaderProgram> program();
    std::vector<std::shared_ptr<Renderer>> renders();
    void clear();

private:

    std::shared_ptr<ShaderProgram> _program;
    std::vector<std::shared_ptr<Renderer>> _renders;
    static std::shared_ptr<ShaderRenderNode> _defaultShader;
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: RENDER_GR953KUN */
