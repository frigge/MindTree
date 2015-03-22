#include "random"
#include "glm/gtx/string_cast.hpp"
#include "rsm_computation_plane.h"

using namespace MindTree;
using namespace GL;

struct RSMProgram : public PixelPlane::ShaderProvider {
    std::shared_ptr<ShaderProgram> provideProgram()
    {
        std::shared_ptr<ShaderProgram> prog;
        prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/rsm_indirect_lighting.frag", 
                                ShaderProgram::FRAGMENT);

        return prog;
    }

};

RSMIndirectPlane::RSMIndirectPlane() :
    _intensity(1.f),
    _searchRadius(.5f),
    _numSamples(400),
    _samplesChanged(false)
{
    setProvider<RSMProgram>();
}

void RSMIndirectPlane::init(std::shared_ptr<ShaderProgram> program)
{
    PixelPlane::init(program);
    initSamplingTexture();
}

void RSMIndirectPlane::initSamplingTexture()
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

void RSMIndirectPlane::drawLight(const LightPtr light, std::shared_ptr<ShaderProgram> program)
{
    if(light->getLightType() != Light::SPOT)
        return;

    if(_samplesChanged)
        initSamplingTexture();

    program->setTexture(_samplingPattern);
    UniformStateManager manager(program);

    manager.addState("searchradius", _searchRadius.load());
    manager.addState("intensity", _intensity.load());
    manager.addState("numSamples", _numSamples.load());

    LightAccumulationPlane::drawLight(light, program);
}

void RSMIndirectPlane::setSearchRadius(double radius)
{
    _searchRadius = radius;
}

void RSMIndirectPlane::setIntensity(double intensity)
{
    _intensity = intensity;
}
void RSMIndirectPlane::setSamples(int samples)
{
    if(samples == _numSamples)
        return;

    _numSamples = samples;
    _samplesChanged = true;
}
