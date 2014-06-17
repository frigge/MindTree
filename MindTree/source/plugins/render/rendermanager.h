#ifndef MT_GL_RENDERMANAGER
#define MT_GL_RENDERMANAGER

#include "glm/glm.hpp"
#include "mutex"
#include "thread"
#include "memory"
#include "list"
#include "../datatypes/Object/object.h"

namespace MindTree {
namespace GL {

class RenderPass;

class RenderConfig
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

class RenderManager
{
public:
    RenderManager();
    virtual ~RenderManager();

    void start();
    void stop();
    bool isRendering();

    std::shared_ptr<RenderPass> addPass();
    void removePass(uint index);
    RenderPass* getPass(uint index);
    void setConfig(RenderConfig cfg);
    RenderConfig getConfig();

    void setSize(int width, int height);

private:
    void init();
    void draw();

    std::thread _renderThread;
    std::mutex _managerLock;
    std::mutex _renderingLock;
    glm::vec4 backgroundColor;
    std::vector<std::shared_ptr<RenderPass>> passes;
    RenderConfig config;
    bool _initialized;
    bool _rendering;
    double renderTime;

    int _width, _height;
};

}
}
#endif
