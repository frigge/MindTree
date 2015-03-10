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

private:
    static void start();
    static void stop();
    static bool isRendering();

    static bool _rendering;
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

    std::vector<std::string> getAllOutputs() const;

    void addPass(std::shared_ptr<RenderPass> pass);
    void insertPassBefore(std::weak_ptr<RenderPass> ref_pass, std::shared_ptr<RenderPass> pass);
    void insertPassAfter(std::weak_ptr<RenderPass> ref_pass, std::shared_ptr<RenderPass> pass);
    void removePass(std::weak_ptr<RenderPass> pass);
    RenderPass* getPass(uint index);

    void setConfig(RenderConfig cfg);
    RenderConfig getConfig();
    void setDirty();

    static std::shared_ptr<ResourceManager> getResourceManager();


private:
    void init();
    void draw();
    friend class RenderThread;

    std::mutex _managerLock;

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
