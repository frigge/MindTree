#include "renderpass.h"
#include "render_setup.h"
#include "rendertree.h"
#include "compositor_plane.h"

using namespace MindTree;
using namespace MindTree::GL;

CompositorPlane::CompositorPlane()
    : PixelPlane("../plugins/render/defaultShaders/compositing.frag")
{
}

CompositorPlane::CompositInfo& CompositorPlane::addLayer(Texture2D *tx, float mix, CompositType type)
{
    CompositInfo info;
    info.type = type;
    info.texture = tx;
    info.mixValue = mix;
    _layers.push_back(info);
    return _layers[_layers.size() - 1];
}

void CompositorPlane::draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    int i = -1;
    for(auto layer : _layers) {
        if(!layer.enabled)
            continue;
        program->setUniform("mixValue", layer.mixValue);
        program->setTexture(layer.texture, "layer");
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
            return info.texture->getName() == txName;
        });
}

const std::vector<CompositorPlane::CompositInfo>& CompositorPlane::getLayers() const
{
    return _layers;
}

std::vector<CompositorPlane::CompositInfo>& CompositorPlane::getLayers()
{
    return _layers;
}

Compositor::Compositor() :
    _plane{new CompositorPlane()}
{
}

void Compositor::init()
{
    _pixelPass = addPass("compositing");
    _pixelPass->setBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
    _pixelPass->addRenderer(_plane);
    _pixelPass->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                  "final_out"));

    PropertyMap layer = {
        {"enabled", true},
        {"mixValue", 1.0}
    };

    PropertyMap layerSettings;
    for  (const auto &info : _plane->getLayers()) {
        layerSettings[info.texture->getName()] = layer;
    }

    _config->addSettings("LayerSettings", layerSettings);
}

void Compositor::setProperty(std::string name, Property prop)
{
    std::string layername;
    std::string setting;
    auto pos = name.find(":");

    if (pos != std::string::npos) {
        layername = name.substr(0, pos);
        setting = name.substr(pos + 1, std::string::npos);
    }

    auto &layers = _plane->getLayers();
    auto it = std::find_if(begin(layers),
                           end(layers),
                           [&name=layername] (const CompositorPlane::CompositInfo &info) {
                               return info.texture->getName() == name;
                           });
    if (it != end(layers)) {
        if(setting == "enabled")
            it->enabled = prop.getData<bool>();
        else if(setting == "mixValue")
            it->mixValue = prop.getData<double>();
    } else if (name == "GL:CUSTOMPIXELSHADER") {
        auto &layers = _plane->getLayers();
        auto custompass =  std::find_if(begin(layers),
                                        end(layers),
                                        [](const auto &info) {
                                            return info.texture->getName() == "custom_pixelshader";
                                        });
        std::string shader = prop.getData<std::string>();
        if (custompass == end(layers)) {
            custom_pass_ = addPassBefore(_pixelPass, "customPixelShader");
            auto *plane = new PixelPlane();
            plane->setFragmentShader(shader);
            custom_pass_->addRenderer(plane);
            auto texture = make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                    "custom_pixelshader");
            auto *tx = texture.get();
            custom_pass_->addOutput(std::move(texture));
            addLayer(tx, 1.0, CompositorPlane::CompositType::ALPHAOVER);
        }
        else {
            custom_pass_->clearRenderers();
            auto *plane = new PixelPlane();
            plane->setFragmentShader(shader);
            custom_pass_->addRenderer(plane);
        }
    }
}

void Compositor::addLayer(Texture2D *tx,
                          float mix,
                          CompositorPlane::CompositType type)
{
    _plane->addLayer(tx, mix, type);
}

std::vector<std::string> Compositor::getLayerNames() const
{
    auto layers = _plane->getLayers();
    std::vector<std::string> names;
    for (const auto &layer : layers) {
        names.push_back(layer.texture->getName());
    }
    return names;
}
