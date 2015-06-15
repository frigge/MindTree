#include "renderpass.h"
#include "pixel_plane.h"

#include "ambient_occlusion.h"

using namespace MindTree;
using namespace MindTree::GL;

AmbientOcclusionPlane::AmbientOcclusionPlane() :
    _searchRadius(.5f),
    _numSamples(16),
    _samplesChanged(false),
    _ambientColor(0.5, 0.5, 0.5, 1.0)
{
}

void AmbientOcclusionPlane::init(std::shared_ptr<ShaderProgram> prog)
{
    PixelPlane::init(prog);
    initSamplingTexture();
}

void AmbientOcclusionPlane::initSamplingTexture()
{
    _samplingPattern = std::make_shared<Texture>("samplingPattern", Texture::RG32F);

    size_t dataSize = _numSamples.load();
    std::vector<glm::vec2> samples;
    std::mt19937 engine;
    std::uniform_real_distribution<float> uniform_distribution;
    for(int i = 0; i< dataSize; ++i) {
        samples.emplace_back(uniform_distribution(engine),
                             uniform_distribution(engine));
    }

    _samplingPattern->setWidth(_numSamples.load());
    _samplingPattern->init(samples);
    _samplesChanged = false;
}

void AmbientOcclusionPlane::setAmbientColor(glm::vec4 color)
{
    std::lock_guard<std::mutex> lock(_colorLock);
    _ambientColor = color;
}

void AmbientOcclusionPlane::setSamples(int samples)
{
    if(samples != _numSamples)
        _samplesChanged = true;

    _numSamples = samples;
}

void AmbientOcclusionPlane::setSearchRadius(float radius)
{
    _searchRadius = radius;
}

void AmbientOcclusionPlane::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    if(_samplesChanged)
        initSamplingTexture();

    std::lock_guard<std::mutex> lock(_colorLock);

    program->setTexture(_samplingPattern);
    UniformStateManager manager(program);

    manager.addState("searchradius", _searchRadius.load());
    manager.addState("numSamples", _numSamples.load());
    manager.addState("ambient_color", _ambientColor);

    PixelPlane::draw(camera, config, program);
}

struct AOShader : public PixelPlane::ShaderProvider {
    std::shared_ptr<ShaderProgram> provideProgram()
    {
        auto prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert",
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/ambient_occlusion.frag",
                                ShaderProgram::FRAGMENT);

        return prog;
    }
};

AmbientOcclusionBlock::AmbientOcclusionBlock() :
    _aoplane(nullptr)
{
}

void AmbientOcclusionBlock::init()
{
    auto aopass = addPass();
    auto aoplane = new AmbientOcclusionPlane();
    aoplane->setProvider<AOShader>();
    aopass->addRenderer(aoplane);

    aopass->addOutput(std::make_shared<Texture2D>("ambient_occlusion", Texture::RGBA16F));
}

void AmbientOcclusionBlock::setGeometry(std::shared_ptr<Group> grp)
{
    if(grp->hasProperty("AO:enabled")) {
        auto enabled = grp->getProperty("AO:enabled").getData<bool>();
        setEnabled(enabled);
        if(!enabled) {
            return;
        }
    }

    if(grp->hasProperty("AO:numsamples")) {
        _aoplane->setSamples(grp->getProperty("AO:numsamples").getData<int>());
    }

    if(grp->hasProperty("AO:searchradius")) {
        _aoplane->setSearchRadius(grp->getProperty("AO:searchradius").getData<double>());
    }

    if(grp->hasProperty("AO:ambientcolor")) {
        _aoplane->setAmbientColor(grp->getProperty("AO:ambientcolor").getData<glm::vec4>());
    }
}
