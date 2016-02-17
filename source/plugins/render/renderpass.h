#ifndef MT_GL_RENDERPASS_H
#define MT_GL_RENDERPASS_H

#include "GL/glew.h"
#include "memory"
#include "mutex"
#include "shared_mutex"
#include "vector"
#include "queue"
#include "utility"

#include "glwrapper.h"

#include "../datatypes/Object/object.h"
#include "../datatypes/Object/lights.h"
#include "data/mtobject.h"

namespace MindTree {
class Benchmark;

namespace GL {

class ShaderRenderNode;

class Texture2D;
class Renderer;
class VAO;
class FBO;
class Renderbuffer;
class RenderConfig;
class ShaderProgram;
class RenderTree;

class RenderPass : public Object
{
public:
    RenderPass();
    virtual ~RenderPass();

    void setBenchmark(std::shared_ptr<Benchmark> benchmark);
    void setCamera(CameraPtr camera);

    void setTarget(ResourceHandle<FBO> &&target);
    FBO* getTarget();

    void addPostRenderCallback(std::function<void(RenderPass*)> cb);

    enum DepthOutput {
        TEXTURE,
        RENDERBUFFER,
        NONE
    };

    void setDepthOutput(ResourceHandle<Texture2D> &&output);
    void setDepthOutput(ResourceHandle<Renderbuffer> &&output);

    void addOutput(ResourceHandle<Texture2D> &&tex);
    void addOutput(ResourceHandle<Renderbuffer> &&rb);

    std::vector<Texture2D*> getOutputTextures();
    Texture2D* getOutDepthTexture();

    std::vector<std::shared_ptr<ShaderRenderNode>> getShaderNodes();

    void setBackgroundColor(glm::vec4 color);
    void setClearDepth(float value);

    void setBlendFunc(GLenum src, GLenum dst);
    void setBlendFuncSeparate(GLenum srcColor, GLenum srcAlpha, GLenum dstColor, GLenum dstAlpha);
    void setEnableBlending(bool value);

    CameraPtr getCamera();

    std::vector<glm::vec4> readPixel(std::vector<std::string> name, glm::ivec2 pos);

    void setOverrideProgram(ResourceHandle<ShaderProgram> &&program);

    void addShaderNode(std::shared_ptr<ShaderRenderNode> node);
    void addGeometryShaderNode(std::shared_ptr<ShaderRenderNode> node);

    void addRenderer(Renderer *renderer);
    void addGeometryRenderer(Renderer *renderer);

    void clearRenderers();
    void clearUnusedShaderNodes();
    void setTextures(std::vector<Texture2D*> textures);
    void setCustomTextureNameMapping(std::string realname, std::string newname);
    void clearCustomTextureNameMapping();
    void setCustomFragmentNameMapping(std::string realname, std::string newname);
    std::string getFragmentName(Texture2D *tex) const;
    void clearCustomFragmentNameMapping();

    void setTree(RenderTree *tree);

    void setEnabled(bool enable);
    bool isEnabled() const;

private:
    void init();
    void render(const RenderConfig &config);
    void setDirty();

    void processPixelRequests();
    void addShaderNodeNoLock(std::shared_ptr<ShaderRenderNode> node);
    void addGeometryShaderNodeNoLock(std::shared_ptr<ShaderRenderNode> node);
    std::pair<bool, std::shared_ptr<ShaderRenderNode>>
        findNodeToInsert(Renderer *renderer,
                         std::vector<std::shared_ptr<ShaderRenderNode>> nodes);

    std::string getTextureName(Texture2D *tex) const;

    std::vector<glm::vec4> _requestedPixels;
    std::queue<std::pair<std::string, glm::ivec2>> _pixelRequests;
    std::shared_timed_mutex _pixelRequestsLock;

    friend class RenderTree;

    std::atomic<bool> _initialized;
    std::atomic<bool> _enabled;
    std::shared_ptr<Camera> _camera;
    ResourceHandle<FBO> _target;

    std::shared_timed_mutex _geometryLock;
    std::shared_timed_mutex _shapesLock;
    std::shared_timed_mutex _cameraLock;

    std::vector<std::shared_ptr<ShaderRenderNode>> _shadernodes;
    std::vector<std::shared_ptr<ShaderRenderNode>> _geometryShaderNodes;

    std::vector<ResourceHandle<Texture2D>> _outputTextures;
    std::vector<ResourceHandle<Renderbuffer>> _outputRenderbuffers;

    ResourceHandle<Texture2D> _depthTexture;
    ResourceHandle<Renderbuffer> _depthRenderbuffer;

    std::shared_timed_mutex _blendLock;
    GLenum _blendColorSource;
    GLenum _blendAlphaSource;
    GLenum _blendColorDest;
    GLenum _blendAlphaDest;

    std::atomic<bool> _blending;

    DepthOutput _depthOutput;

    std::mutex _bgColorLock;
    glm::vec4 _bgColor;

    std::atomic<float> _depth;

    std::shared_timed_mutex _overrideProgramLock;
    ResourceHandle<ShaderProgram> _overrideProgram;
    std::atomic_bool overrideProgramFlag_;

    int _currentWidth, _currentHeight;

    mutable std::shared_timed_mutex _textureNameMappingLock;
    std::unordered_map<std::string, std::string> _textureNameMappings;

    mutable std::shared_timed_mutex _fragmentNameMappingLock;
    std::unordered_map<std::string, std::string> _fragmentNameMappings;

    RenderTree *_tree;

    std::shared_ptr<Benchmark> _benchmark;

    std::vector<std::function<void(RenderPass*)>> _postRenderCallbacks;
};

}
}
#endif
