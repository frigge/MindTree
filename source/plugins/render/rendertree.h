#ifndef MT_GL_RENDERMANAGER
#define MT_GL_RENDERMANAGER

#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp"
#include "mutex"
#include "shared_mutex"

#include "atomic"
#include "thread"
#include "memory"
#include "queue"
#include "unordered_map"
#include "../datatypes/Object/object.h"

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
class ResourceManager;
class RenderTree
{
public:
    RenderTree();
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
    void draw();

private:
    void init();

    std::shared_timed_mutex _managerLock;

    glm::vec4 backgroundColor;
    std::unique_ptr<ResourceManager> _resourceManager;
    std::vector<std::unique_ptr<RenderPass>> passes;
    RenderConfig config;
    std::atomic_bool _initialized;
    double renderTime;

    std::shared_ptr<Benchmark> _benchmark;
};

}
}
#endif
