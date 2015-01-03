#ifndef MT_GL_RENDERMANAGER
#define MT_GL_RENDERMANAGER

#include "glm/glm.hpp"
#include "mutex"
#include "thread"
#include "memory"
#include "queue"
#include "unordered_map"
#include "../datatypes/Object/object.h"

class QGLContext;

namespace MindTree {
namespace GL {

class RenderPass;

class RenderConfig : public Object
{
public:
    void setDrawPoints(bool draw);
    void setDrawEdges(bool draw);
    void setDrawPolygons(bool draw);
    void setShowFlatShaded(bool b);
    bool drawPoints() const;
    bool drawEdges() const;
    bool drawPolygons() const;
    bool flatShading() const;

private:
    bool _drawPoints = true;
    bool _drawEdges = true;
    bool _drawPolygons = true;
    bool _flatShading = false;
};

class RenderManager;
class RenderThread
{
public:
    static void addManager(RenderManager* manager);
    static void removeManager(RenderManager *manager);
    inline static std::thread::id id() { return _renderThread.get_id(); }
    inline static void asrt() { assert(_renderThread.get_id() == std::this_thread::get_id()); }

private:
    static void start();
    static void stop();
    static bool isRendering();

    static bool _rendering;
    static std::mutex _renderingLock;
    static std::thread _renderThread;
    static std::vector<RenderManager*> _renderQueue;
};

class ResourceManager;
class RenderManager
{
public:
    RenderManager(QGLContext *context);
    virtual ~RenderManager();

    void setCustomTextureNameMapping(std::string realname, std::string newname);
    void clearCustomTextureNameMapping();
    std::vector<std::string> getAllOutputs() const;

    void removePass(uint index);
    RenderPass* getPass(uint index);
    void setConfig(RenderConfig cfg);
    RenderConfig getConfig();
    void setDirty();

    template<typename T>
    std::shared_ptr<T> addPass()
    {
        std::lock_guard<std::mutex> lock(_managerLock);
        auto pass = std::make_shared<T>();
        passes.push_back(pass);
        return pass;
    }

    static std::shared_ptr<ResourceManager> getResourceManager();


private:
    void init();
    void draw();
    friend class RenderThread;

    std::mutex _managerLock;
    std::unordered_map<std::string, std::string> _textureNameMappings;

    glm::vec4 backgroundColor;
    std::vector<std::shared_ptr<RenderPass>> passes;
    static std::shared_ptr<ResourceManager> _resourceManager;
    RenderConfig config;
    QGLContext *_context;
    bool _initialized;
    double renderTime;
};

}
}
#endif
