#include "renderpass.h"
#include "render_setup.h"
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
    CompositInfo info;
    info.type = type;
    info.texture = tx;
    info.mixValue = mix;
    _layers.push_back(info);
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
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            break;
        case CompositType::MULTIPLY:
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            //glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_DST_ALPHA, GL_ZERO);
            glBlendEquation(GL_FUNC_ADD);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            break;
        case CompositType::MIX:
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFunc(GL_ONE, GL_ONE);
            //glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            break;
        case CompositType::ALPHAOVER:
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
            glBlendEquation(GL_FUNC_ADD);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            break;
        }
        PixelPlane::draw(camera, config, program);
        ++i;
    }
}

CompositorPlane::CompositInfo& CompositorPlane::getInfo(std::string txName)
{
    return *std::find_if(begin(_layers), end(_layers), [&txName] (const CompositInfo &info) {
            return info.texture.lock()->getName() == txName;
        });
}

const std::vector<CompositorPlane::CompositInfo>& CompositorPlane::getLayers() const
{
    return _layers;
}

Compositor::Compositor() :
    _plane{new CompositorPlane()}
{
}

void Compositor::init()
{
    auto pixelPass = addPass();
    _pixelPass = pixelPass.get();
    pixelPass->setBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
    pixelPass->addRenderer(_plane);
    pixelPass->addOutput(std::make_shared<Texture2D>("final_out"));

    PropertyMap layer = {
        {"enabled", true},
        {"mixValue", 1.0}
    };

    PropertyMap layerSettings;
    for  (const auto &info : _plane->getLayers()) {
        layerSettings[info.texture.lock()->getName()] = layer;
    }

    _config->addSettings("LayerSettings", layerSettings);
}

void Compositor::setProperty(std::string name, Property prop)
{
}

void Compositor::addLayer(std::weak_ptr<Texture2D> tx, float mix, CompositorPlane::CompositType type)
{
    _plane->addLayer(tx, mix, type);
}
