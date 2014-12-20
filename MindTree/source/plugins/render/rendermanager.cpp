#include "GL/glew.h"
#include "QGLContext"
#include "glm/gtc/matrix_transform.hpp"
#include "glwrapper.h"
#include "chrono"
#include "render.h"
#include "renderpass.h"
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

bool RenderThread::_rendering = false;
std::mutex RenderThread::_renderingLock;
std::thread RenderThread::_renderThread;
std::vector<RenderManager*> RenderThread::_renderQueue;

void RenderThread::addManager(RenderManager* manager)
{
    auto it = std::find(begin(_renderQueue), end(_renderQueue), manager);
    if(it == end(_renderQueue))
        _renderQueue.push_back(manager);

    if(!isRendering()) start();
}

void RenderThread::removeManager(RenderManager *manager)
{
    auto it = std::find(begin(_renderQueue), end(_renderQueue), manager);
    if(it != end(_renderQueue))
        _renderQueue.erase(it);
    if(_renderQueue.empty()) stop();
}

bool RenderThread::isRendering()
{
    std::lock_guard<std::mutex> lock(_renderingLock);
    return _rendering;
}

void RenderThread::start()
{
    if(isRendering()) stop();

    _rendering = true;

    auto renderLoop = [] {
        while(RenderThread::isRendering()) {
            for(auto *manager : _renderQueue) {
                manager->draw();
            }
        }
    };

    _renderThread = std::thread(renderLoop);
}

void RenderThread::stop()
{
    std::cout << "stop rendering" << std::endl;
    {
        std::lock_guard<std::mutex> lock(_renderingLock);
        _rendering = false;
    }
    if (_renderThread.joinable()) _renderThread.join();
}

std::shared_ptr<ResourceManager> RenderManager::_resourceManager;

RenderManager::RenderManager(QGLContext *context)
    : _initialized(false), _context(context)
{
    if(!_resourceManager) _resourceManager = std::make_shared<ResourceManager>();
}

RenderManager::~RenderManager()
{
}

std::shared_ptr<ResourceManager> RenderManager::getResourceManager()
{
    return _resourceManager;
}

void RenderManager::setDirty()
{
    std::lock_guard<std::mutex> lock(_managerLock);
    _initialized = false;
}

void RenderManager::setCustomTextureNameMapping(std::string realname, std::string newname)
{
    {
        std::lock_guard<std::mutex> lock(_managerLock);
        _textureNameMappings[realname] = newname;
    }
    setDirty();
}

void RenderManager::clearCustomTextureNameMapping()
{
    {
        std::lock_guard<std::mutex> lock(_managerLock);
        _textureNameMappings.clear();
    }
    setDirty();
}

void RenderManager::init()
{
    RenderThread::asrt();
    if(_initialized) return;

    static bool glewInitialized = false;

    if(!glewInitialized) glewInit();

    _initialized = true;
    glClearColor( 0., 0., 0., 0. );

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //connect output textures to all following passes
    uint i=0;
    for(auto &pass : passes){
        pass->init();
        if(i > 0) {
            for (uint j = 0; j < i; ++j){
                auto lastPass = passes[j];
                auto textures = lastPass->getOutputTextures();

                auto shadernodes = pass->getShaderNodes();
                for (auto shadernode : shadernodes) {
                    for(auto texture : textures) {
                        shadernode->program()->setTexture(texture, _textureNameMappings[texture->getName()]);
                    }
                    if(lastPass->_depthOutput == RenderPass::TEXTURE)
                        shadernode->program()->setTexture(lastPass->getOutDepthTexture());
                }
            }
        }
        ++i;
    }
}

std::vector<std::string> RenderManager::getAllOutputs() const
{
    std::vector<std::string> outputs;
    for(const auto &pass : passes) {
        auto textures = pass->getOutputTextures();
        for(const auto &tex : textures) {
            outputs.push_back(tex->getName());
        }
    }
    return outputs;
}

void RenderManager::setConfig(RenderConfig cfg)    
{
    config = cfg;
}

RenderConfig RenderManager::getConfig()    
{
    return config;
}

void RenderManager::removePass(uint index)    
{
    std::lock_guard<std::mutex> lock(_managerLock);
}

RenderPass* RenderManager::getPass(uint index)
{
    std::lock_guard<std::mutex> lock(_managerLock);
    return std::next(begin(passes), index)->get();
}

void RenderManager::draw()
{
    ContextBinder binder(_context);
    RenderThread::asrt();

    if(!_initialized) {
        init();
    }
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POLYGON_OFFSET_POINT);

    auto start = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(_managerLock);
        int i = 0;
        for(auto &pass : passes){
            pass->render(config);
        }
    }

    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POLYGON_OFFSET_POINT);
    _context->swapBuffers();

    glFinish();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    renderTime = duration.count() / 1000000000.0;
    //std::cout << "Rendering took " << seconds << "s"
    //    << " ==> " << 1.0 / seconds << "fps" << std::endl;
}
