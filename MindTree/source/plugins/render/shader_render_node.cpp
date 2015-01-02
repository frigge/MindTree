#include "rendermanager.h"
#include "glwrapper.h"
#include "render.h"
#include "shader_render_node.h"

using namespace MindTree;
using namespace MindTree::GL;

ShaderRenderNode::ShaderRenderNode(std::shared_ptr<ShaderProgram> program) : 
    _program(program),
    _initialized(false)
{
}

ShaderRenderNode::~ShaderRenderNode()
{
    if(_program) RenderManager::getResourceManager()
        ->scheduleCleanUp(_program);
}

void ShaderRenderNode::init()
{
    if(_initialized) return;

    RenderThread::asrt();
    _initialized = true;
    _program->init();
    for (auto render : _renders)
        render->_init(_program);
}

void ShaderRenderNode::addRenderer(Renderer *renderer)
{
    _renders.push_back(std::shared_ptr<Renderer>(renderer));
    _initialized = false;
}

void ShaderRenderNode::render(CameraPtr camera, glm::ivec2 resolution, const RenderConfig &config)
{
    RenderThread::asrt();
    if(!_initialized || !_program) return;

    {
        UniformState us(_program, "resolution", resolution);
        for(const auto &renderer : _renders) {
            renderer->render(camera, config, _program);
        }
    }
}

std::shared_ptr<ShaderProgram> ShaderRenderNode::program()
{
    return _program;
}

void ShaderRenderNode::clear()
{
    _renders.clear();
}

const std::vector<std::shared_ptr<Renderer>>& ShaderRenderNode::renders()
{
    return _renders;
}

