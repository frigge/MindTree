#include "rendertree.h"
#include "glwrapper.h"
#include "render.h"
#include "shader_render_node.h"

using namespace MindTree;
using namespace MindTree::GL;

ShaderRenderNode::ShaderRenderNode(ShaderProgram *program) :
    _program(program),
    _persistend(false),
    _initialized(false),
    _resourceManager(nullptr)
{
    assert(_program);
}

void ShaderRenderNode::setResourceManager(ResourceManager *manager)
{
    assert(manager != nullptr);
    _resourceManager = manager;
}

void ShaderRenderNode::setPersistend(bool pers)
{
    _persistend = pers;
}

bool ShaderRenderNode::isPersistend() const
{
    return _persistend;
}

void ShaderRenderNode::init()
{
    if(_initialized) return;
    std::lock_guard<std::mutex> lock(_rendersLock);

    RenderThread::asrt();
    _initialized = true;
    _program->init();
    for (auto &render : _renders)
        render->_init(_program);
}

void ShaderRenderNode::addRenderer(Renderer *renderer)
{
    std::lock_guard<std::mutex> lock(_rendersLock);
    renderer->setResourceManager(_resourceManager);
    _renders.push_back(std::unique_ptr<Renderer>(renderer));
    _initialized = false;
}

void ShaderRenderNode::render(CameraPtr camera, glm::ivec2 resolution, const RenderConfig &config)
{
    RenderThread::asrt();
    std::lock_guard<std::mutex> lock(_rendersLock);
    if(!_initialized || !_program) return;

    {
        UniformState us(_program, "resolution", resolution);
        for(const auto &renderer : _renders) {
            renderer->render(camera, config, _program);
        }
    }
}

ShaderProgram* ShaderRenderNode::program()
{
    return _program;
}

void ShaderRenderNode::clear()
{
    std::lock_guard<std::mutex> lock(_rendersLock);
    _renders.clear();
}

std::vector<Renderer*> ShaderRenderNode::renders()
{
    std::lock_guard<std::mutex> lock(_rendersLock);
    std::vector<Renderer*> ret;
    for(auto &render : _renders)
        ret.push_back(render.get());
    return ret;
}
