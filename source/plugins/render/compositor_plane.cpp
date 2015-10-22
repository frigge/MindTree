#include "compositor_plane.h"

using namespace MindTree;
using namespace MindTree::GL;
template<>
const std::string
PixelPlane::ShaderFiles<CompositorPlane>::
fragmentShader = "../plugins/render/defaultShaders/compositing.frag";

CompositorPlane::CompositorPlane()
{
    setProvider<CompositorPlane>();
}

CompositorPlane::CompositInfo& CompositorPlane::addLayer(std::weak_ptr<Texture2D> tx, float mix, CompositType type)
{
    _layers.push_back({type, mix, tx});
    return _layers[_layers.size() - 1];
}

void CompositorPlane::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    int i = -1;
    for(auto layer : _layers) {
        if(!layer.enabled || layer.texture.expired())
            continue;
        program->setUniform("mixValue", layer.mixValue);
        program->setTexture(layer.texture.lock(), "layer");
        if(i > -1) program->setUniform("last_type", (int)_layers[i].type);
        else program->setUniform("last_type", -1);
        switch(layer.type) {
        case CompositType::ADD:
            glBlendFunc(GL_ONE, GL_ZERO);
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendEquation(GL_FUNC_ADD);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
            break;
        case CompositType::MULTIPLY:
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case CompositType::MIX:
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case CompositType::ALPHAOVER:
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            break;
        }
        PixelPlane::draw(camera, config, program);
        ++i;
    }
}
