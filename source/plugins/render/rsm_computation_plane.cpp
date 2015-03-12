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
    _searchRadius(.5f)
{
    setProvider<RSMProgram>();
}

void RSMIndirectPlane::init(std::shared_ptr<ShaderProgram> program)
{
    PixelPlane::init(program);

    _samplingPattern = std::make_shared<Texture2D>("samplingPattern", Texture::RG);
    
    std::vector<unsigned char> samples(800);
    std::mt19937 engine;
    std::uniform_real_distribution<float> uniform_distribution;
    for(int i = 0; i< 800; i += 2) {
        samples[i] = uniform_distribution(engine) * 255;
        samples[i + 1] = uniform_distribution(engine) * 255;
    }

    _samplingPattern->setWidth(20);
    _samplingPattern->setWidth(20);
    _samplingPattern->initFromData(samples);
}

void RSMIndirectPlane::drawLight(const LightPtr light, std::shared_ptr<ShaderProgram> program) const
{
    if(light->getLightType() != Light::SPOT)
        return;

    program->setTexture(_samplingPattern);
    UniformStateManager manager(program);

    manager.addState("searchradius", _searchRadius.load());
    manager.addState("intensity", _intensity.load());

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
