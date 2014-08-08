#include "GL/glew.h"
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

RenderManager::RenderManager()
    : _initialized(false),
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
                        shadernode->program()->setTexture(texture);
                    }
                    if(lastPass->_depthOutput == RenderPass::TEXTURE)
                        shadernode->program()->setTexture(lastPass->getOutDepthTexture());
                }
            }
        }
        ++i;
    }
}

void RenderManager::setSize(int width, int height)
{
    if(_width == width && _height == height) 
        return;

    _width = width;
    _height = height;

    for (auto pass : passes)
        pass->setSize(_width, _height);

    //invalidate renderpass so that FBO and textures are regenerated
    _initialized = false;
}

void RenderManager::setConfig(RenderConfig cfg)    
{
    config = cfg;
}

RenderConfig RenderManager::getConfig()    
{
    return config;
}

std::shared_ptr<RenderPass> RenderManager::addPass()
{
    std::lock_guard<std::mutex> lock(_managerLock);
    auto pass = std::make_shared<RenderPass>();
    passes.push_back(pass);
    return pass;
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
    SharedContextRAII context;
    if(!_initialized) {
        init();
    }
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POLYGON_OFFSET_POINT);

    auto start = std::chrono::steady_clock::now();
    Context::getSharedContext()->getManager()->cleanUp();
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
    Context::getSharedContext()->swapBuffers();
    glFinish();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    renderTime = duration.count() / 1000000000.0;
    //std::cout << "Rendering took " << seconds << "s"
    //    << " ==> " << 1.0 / seconds << "fps" << std::endl;
}
