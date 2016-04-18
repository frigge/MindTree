#ifndef MT_GL_RENDERMANAGER
#define MT_GL_RENDERMANAGER

#include "glm/glm.hpp"
#include "mutex"
#include "shared_mutex"

#include "atomic"
#include "thread"
#include "memory"
#include "queue"
#include "unordered_map"
#include "../datatypes/Object/object.h"

class QGLContext;

namespace MindTree {
class Benchmark;
namespace GL {

class Texture;
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

class RenderTree;
class RenderThread
{
public:
    static void addManager(RenderTree* manager);
    static void removeManager(RenderTree *manager);
    inline static std::thread::id id() { return _renderThread.get_id(); }
    inline static void asrt() { assert(_renderThread.get_id() == std::this_thread::get_id()); }
    static void update();
    static void updateOnce();
    static void pause();

private:
    static void start();
    static void stop();
    static bool isRendering();

    static std::atomic_bool _rendering;
    static std::atomic_bool _update;
    static std::condition_variable _renderNotifier;
    static std::mutex _renderingLock;
    static std::thread _renderThread;
    static std::vector<RenderTree*> _renderQueue;
};

class ResourceManager;
class RenderTree
{
public:
    RenderTree(QGLContext *context);
    virtual ~RenderTree();

    void setBenchmark(std::shared_ptr<Benchmark> benchmark);
    std::weak_ptr<Benchmark> getBenchmark() const;

    std::vector<std::string> getAllOutputs() const;

    void addPass(std::unique_ptr<RenderPass> &&pass);
    void insertPassBefore(const RenderPass *ref_pass, std::unique_ptr<RenderPass> &&pass);
    void insertPassAfter(const RenderPass *ref_pass, std::unique_ptr<RenderPass> &&pass);
    void removePass(RenderPass *pass);
    RenderPass* getPass(uint index);

    void setConfig(RenderConfig cfg);
    RenderConfig getConfig();
    void setDirty();

    ResourceManager *getResourceManager();


private:
    void init();
    void draw();
    friend class RenderThread;

    std::shared_timed_mutex _managerLock;

    glm::vec4 backgroundColor;
    std::unique_ptr<ResourceManager> _resourceManager;
    std::vector<std::unique_ptr<RenderPass>> passes;
    RenderConfig config;
    QGLContext *_context;
    std::atomic_bool _initialized;
    double renderTime;

    std::shared_ptr<Benchmark> _benchmark;
};

}
}
#endif
