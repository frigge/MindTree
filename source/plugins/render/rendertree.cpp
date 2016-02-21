#include "GL/glew.h"
#include "QGLContext"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"
#include "chrono"
#include "render.h"
#include "renderpass.h"
#include "shader_render_node.h"
#include "benchmark.h"
#include "rendertree.h"

using namespace MindTree;
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

std::atomic_bool RenderThread::_rendering{false};
std::atomic_bool RenderThread::_update{false};
std::mutex RenderThread::_renderingLock;
std::condition_variable RenderThread::_renderNotifier;
std::thread RenderThread::_renderThread;
std::vector<RenderTree*> RenderThread::_renderQueue;

void RenderThread::addManager(RenderTree* manager)
{
    std::unique_lock<std::mutex> lock(_renderingLock, std::defer_lock);
    auto it = std::find(begin(_renderQueue), end(_renderQueue), manager);
    if(it == end(_renderQueue))
        _renderQueue.push_back(manager);

    if(!isRendering()) start();
    _update = true;
}

void RenderThread::removeManager(RenderTree *manager)
{
    std::unique_lock<std::mutex> lock(_renderingLock, std::defer_lock);
    auto it = std::find(begin(_renderQueue), end(_renderQueue), manager);
    if(it != end(_renderQueue))
        _renderQueue.erase(it);
    if(_renderQueue.empty()) stop();
}

bool RenderThread::isRendering()
{
    return _rendering;
}

void RenderThread::updateOnce()
{
    _update = true;
    _renderNotifier.notify_all();
    _update = false;
}

void RenderThread::update()
{
    _update = true;
    _renderNotifier.notify_all();
}

void RenderThread::pause()
{
    _update = false;
}

void RenderThread::start()
{
    if(isRendering()) stop();

    _rendering = true;

    auto renderLoop = [] {
        std::unique_lock<std::mutex> lock(_renderingLock, std::defer_lock);
        while(RenderThread::isRendering()) {
            if(!_update) _renderNotifier.wait(lock);
            for(auto *manager : _renderQueue) {
                manager->draw();
            }
        }
    };

    std::cout << "starting render thread" << std::endl;
    _renderThread = std::thread(renderLoop);
}

void RenderThread::stop()
{
    std::cout << "stop rendering" << std::endl;
    _rendering = false;
    _update = false;
    _renderNotifier.notify_all();
    if (_renderThread.joinable()) _renderThread.join();
}

RenderTree::RenderTree(QGLContext *context)
    : _initialized(false),
      _context(context),
      _resourceManager(std::make_unique<ResourceManager>())
{
}

RenderTree::~RenderTree()
{
}

void RenderTree::setBenchmark(std::shared_ptr<Benchmark> benchmark)
{
    _benchmark = benchmark;
}

std::weak_ptr<Benchmark> RenderTree::getBenchmark() const
{
    return _benchmark;
}

ResourceManager *RenderTree::getResourceManager()
{
    return _resourceManager.get();
}

void RenderTree::setDirty()
{
    _initialized = false;
}

void RenderTree::init()
{
    RenderThread::asrt();
    if(_initialized) return;

    static bool glewInitialized = false;

    if(!glewInitialized) glewInit();

    _initialized = true;
    glClearColor( 0., 0., 0., 0. );

    glEnable(GL_CULL_FACE);

    //connect output textures to all following passes
    {
        std::shared_lock<std::shared_timed_mutex> lock(_managerLock);
        uint i=0;
        for(auto &pass : passes){
            pass->init();
            if(i > 0) {
                for (uint j = 0; j < i; ++j){
                    auto *lastPass = passes[j].get();
                    auto textures = lastPass->getOutputTextures();
                    if(lastPass->_depthOutput == RenderPass::TEXTURE)
                        textures.push_back(lastPass->_depthTexture.get());
                    pass->setTextures(textures);

                }
            }
            ++i;
        }
    }
}

std::vector<std::string> RenderTree::getAllOutputs() const
{
    std::vector<std::string> outputs;
    for(const auto &pass : passes) {
        auto textures = pass->getOutputTextures();
        for(const auto &tex : textures) {
            outputs.push_back(tex->getName());
        }
        auto depth = pass->getOutDepthTexture();
        if(depth)
            outputs.push_back(depth->getName());
    }
    return outputs;
}

void RenderTree::setConfig(RenderConfig cfg)
{
    std::lock_guard<std::shared_timed_mutex> lock(_managerLock);
    config = cfg;
}

RenderConfig RenderTree::getConfig()
{
    std::shared_lock<std::shared_timed_mutex> lock(_managerLock);
    return config;
}

void RenderTree::addPass(std::unique_ptr<RenderPass> &&pass)
{
    std::lock_guard<std::shared_timed_mutex> lock(_managerLock);
    pass->setTree(this);
    _initialized = false;
    passes.push_back(std::move(pass));
}

void RenderTree::insertPassBefore(RenderPass *ref_pass, std::unique_ptr<RenderPass> &&pass)
{
    std::lock_guard<std::shared_timed_mutex> lock(_managerLock);
    pass->setTree(this);
    _initialized = false;
    auto it = std::find_if(begin(passes),
                           end(passes),
                           [&p=ref_pass] (const std::unique_ptr<RenderPass> &pass) { return pass.get() == p; });
    passes.insert(it, std::move(pass));
}

void RenderTree::insertPassAfter(RenderPass *ref_pass, std::unique_ptr<RenderPass> &&pass)
{
    std::lock_guard<std::shared_timed_mutex> lock(_managerLock);
    pass->setTree(this);
    _initialized = false;
    auto it = std::find_if(begin(passes),
                           end(passes),
                           [&p=ref_pass] (const std::unique_ptr<RenderPass> &pass) { return pass.get() == p; });
    ++it;
    passes.insert(it, std::move(pass));
}

void RenderTree::removePass(RenderPass *pass)
{
    std::lock_guard<std::shared_timed_mutex> lock(_managerLock);
    auto it = std::find_if(begin(passes),
                           end(passes),
                           [&p=pass] (const std::unique_ptr<RenderPass> &pass) { return pass.get() == p; });
    if(it != passes.end()) {
        passes.erase(it);
        _initialized = false;
    }
    else {
        std::cout << "could not remove renderpass" << std::endl;
    }
}

RenderPass* RenderTree::getPass(uint index)
{
    std::shared_lock<std::shared_timed_mutex> lock(_managerLock);
    return std::next(begin(passes), index)->get();
}

void RenderTree::draw()
{
    ContextBinder binder(_context);
    RenderThread::asrt();

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POLYGON_OFFSET_POINT);

    BenchmarkHandler handler(_benchmark);

    if(!_initialized) {
        init();
    }
    {
        std::shared_lock<std::shared_timed_mutex> lock(_managerLock);
        int i = 0;
        for(auto &pass : passes){
            pass->render(config);
        }
        _resourceManager->cleanUp();
    }

    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POLYGON_OFFSET_POINT);
    _context->swapBuffers();

    glFinish();
}
