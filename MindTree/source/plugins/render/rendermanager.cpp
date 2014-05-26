#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"
#include "chrono"
#include "render.h"
#include "rendermanager.h"

using namespace MindTree::GL;

void RenderConfig::setDrawPoints(bool draw)    
{
    _drawPoints = draw;
}

void RenderConfig::setDrawEdges(bool draw)    
{
    _drawEdges = draw;
}

void RenderConfig::setDrawPolygons(bool draw)    
{
    _drawPolygons = draw;
}

void RenderConfig::setShowFlatShaded(bool b)
{
    _flatShading = b;
}

bool RenderConfig::drawPoints()    const 
{
    return _drawPoints;
}

bool RenderConfig::drawEdges()    const 
{
    return _drawEdges;
}

bool RenderConfig::drawPolygons()    const 
{
    return _drawPolygons;
}

bool RenderConfig::flatShading() const
{
    return _flatShading;
}

RenderManager::RenderManager()
    : backgroundColor(glm::vec4(.275, .275, .275, 1.)),
    _initialized(false),
    _rendering(false)
{
}

RenderManager::~RenderManager()
{
}

bool RenderManager::isRendering()
{
    std::lock_guard<std::mutex> lock(_renderingLock);
    return _rendering;
}

void RenderManager::start()
{
    if(isRendering()) stop();

    _rendering = true;

    auto renderLoop = [this] {
        while(this->isRendering()) {
            this->draw();
        }
    };

    _renderThread = std::thread(renderLoop);
}

void RenderManager::stop()
{
    {
        std::lock_guard<std::mutex> lock(_renderingLock);
        _rendering = false;
    }
    _renderThread.join();
}

void RenderManager::init()
{
    if(_initialized) return;

    _initialized = true;
    glewInit();
    glClearColor(backgroundColor.r, 
                 backgroundColor.g, 
                 backgroundColor.b, 
                 backgroundColor.a);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderManager::setConfig(RenderConfig cfg)    
{
    config = cfg;
}

RenderConfig RenderManager::getConfig()    
{
    return config;
}

RenderPass* RenderManager::addPass()
{
    auto pass = new RenderPass();
    std::lock_guard<std::mutex> lock(_managerLock);
    passes.push_back(std::unique_ptr<RenderPass>(pass));
    return pass;
}

void RenderManager::removePass(uint index)    
{
    std::lock_guard<std::mutex> lock(_managerLock);
    passes.remove(*std::next(passes.begin(), index));
}

RenderPass* RenderManager::getPass(uint index)
{
    std::lock_guard<std::mutex> lock(_managerLock);
    return std::next(begin(passes), index)->get();
}

void RenderManager::draw()
{
    SharedContextRAII context;
    if(!_initialized) {
        init();
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    glEnable(GL_POINT_SMOOTH);

    auto start = std::chrono::steady_clock::now();
    Context::getSharedContext()->getManager()->cleanUp();
    {
        std::lock_guard<std::mutex> lock(_managerLock);
        for(auto &pass : passes){
            pass->render(config);
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    double seconds = duration.count() / 1000000000.0;
    std::cout << "Rendering took " << seconds << "s"
        << " ==> " << 1.0 / seconds << "fps" << std::endl;
    glFinish();
    glDisable(GL_POINT_SMOOTH);
    Context::getSharedContext()->swapBuffers();
}
