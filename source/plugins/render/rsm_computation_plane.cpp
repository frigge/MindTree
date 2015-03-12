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

RSMIndirectPlane::RSMIndirectPlane()
{
    setProvider<RSMProgram>();
}

void RSMIndirectPlane::init(std::shared_ptr<ShaderProgram> program)
{
    PixelPlane::init(program);

    _samplingPattern = std::make_shared<Texture2D>("samplingPattern", Texture::RG16F);
    
    std::vector<glm::vec2> samples(400);
    std::mt19937 engine;
    std::normal_distribution<float> gauss_distribution(0, 1 / sqrt(2));
    std::uniform_real_distribution<float> uniform_distribution;
    for(int i = 0; i< 400; i++) {
        float gauss = gauss_distribution(engine);
        samples[i] = glm::vec2(uniform_distribution(engine), std::abs(gauss_distribution(engine)));
    }

    _samplingPattern->setWidth(20);
    _samplingPattern->setWidth(20);
    _samplingPattern->initFromData(samples);
    _samplingPattern->init();
}

void RSMIndirectPlane::drawLight(const LightPtr light, std::shared_ptr<ShaderProgram> program) const
{
    if(light->getLightType() != Light::SPOT)
        return;

    program->setTexture(_samplingPattern);
    //UniformStateManager manager(program);
    //manager.addState("searchradius", 4);

    LightAccumulationPlane::drawLight(light, program);
}
