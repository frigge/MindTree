#include "renderpass.h"
#include "pixel_plane.h"

#include "ambient_occlusion.h"

using namespace MindTree;
using namespace MindTree::GL;

AmbientOcclusionPlane::AmbientOcclusionPlane() :
    _searchRadius(.5f),
    _numSamples(400),
    _samplesChanged(false),
    _ambientColor(1)
{
}

void AmbientOcclusionPlane::init(std::shared_ptr<ShaderProgram> prog)
{
    PixelPlane::init(prog);
    initSamplingTexture();
}

void AmbientOcclusionPlane::initSamplingTexture()
{
    _samplingPattern = std::make_shared<Texture>("samplingPattern", Texture::RG8);

    static const int components = 2;
    
    size_t dataSize = _numSamples.load() * components;
    std::vector<unsigned char> samples;
    std::mt19937 engine;
    std::uniform_real_distribution<float> uniform_distribution;
    for(int i = 0; i< dataSize; i += components) {
        samples.push_back(uniform_distribution(engine) * 255);
        samples.push_back(uniform_distribution(engine) * 255);
    }

    _samplingPattern->setWidth(_numSamples.load());
    _samplingPattern->init(samples);
    _samplesChanged = false;
}

void AmbientOcclusionPlane::setAmbientColor(glm::vec4 color)
{
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
