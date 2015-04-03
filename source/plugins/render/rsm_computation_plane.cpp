#include "random"
#include "glm/gtx/string_cast.hpp"
#include "render_setup.h"
#include "rendertree.h"
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

struct RSMInterpolateProvider : public PixelPlane::ShaderProvider {
    std::shared_ptr<ShaderProgram> provideProgram()
    {
        std::shared_ptr<ShaderProgram> prog;
        prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/rsm_interpolate.frag", 
                                ShaderProgram::FRAGMENT);

        return prog;
    }

};

struct RSMFinalProvider : public PixelPlane::ShaderProvider {
    std::shared_ptr<ShaderProgram> provideProgram()
    {
        std::shared_ptr<ShaderProgram> prog;
        prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/rsm_final.frag", 
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

RSMGenerationBlock::RSMGenerationBlock()
{
}

std::weak_ptr<RenderPass> RSMGenerationBlock::createShadowPass(SpotLightPtr spot)
{
    auto shadowPass = ShadowMappingRenderBlock::createShadowPass(spot).lock();

    if(!shadowPass)
        return std::weak_ptr<RenderPass>();

    auto pos = std::make_shared<Texture2D>("shadow_position",
                                                             Texture::RGBA16F);
    pos->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(pos);
    auto normal = std::make_shared<Texture2D>("shadow_normal",
                                                             Texture::RGBA16F);
    normal->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(normal);
    auto flux = std::make_shared<Texture2D>("shadow_flux", Texture::RGBA16F);
    shadowPass->addOutput(flux);
    flux->setWrapMode(Texture::REPEAT);

    return shadowPass;
}

void RSMGenerationBlock::setGeometry(std::shared_ptr<Group> grp)
{
    ShadowMappingRenderBlock::setGeometry(grp);

    if(grp->hasProperty("RSM:enabled")) {
        setEnabled(grp->getProperty("RSM:enabled").getData<bool>());
    }
}

RSMEvaluationBlock::RSMEvaluationBlock(RSMGenerationBlock *shadowBlock) :
    _shadowBlock(shadowBlock)
{
}

void RSMEvaluationBlock::init()
{
    auto rsmIndirectLowResPass = addPass();
    _rsmIndirectLowResPass = rsmIndirectLowResPass;
    rsmIndirectLowResPass->addOutput(std::make_shared<Texture2D>("rsm_indirect_out_lowres",
                                                Texture::RGB16F));
    rsmIndirectLowResPass->addOutput(std::make_shared<Texture2D>("normal_lowres",
                                                Texture::RGB16F));
    rsmIndirectLowResPass->addOutput(std::make_shared<Texture2D>("position_lowres",
                                                Texture::RGB16F));
    rsmIndirectLowResPass->setCustomFragmentNameMapping("rsm_indirect_out_lowres", "rsm_indirect_out");

    _rsmIndirectLowResPlane = new RSMIndirectPlane();
    rsmIndirectLowResPass->addRenderer(_rsmIndirectLowResPlane);
    rsmIndirectLowResPass->setBlendFunc(GL_ONE, GL_ONE);

    auto rsmInterpolatePass = addPass();
    rsmInterpolatePass->addOutput(std::make_shared<Texture2D>("rsm_indirect_out_interpolated", Texture::RGBA16F));
    auto rsmInterpolatePlane = new PixelPlane();
    rsmInterpolatePlane->setProvider<RSMInterpolateProvider>();
    rsmInterpolatePass->addRenderer(rsmInterpolatePlane);

    auto rsmIndirectHighResPass = addPass();
    _rsmIndirectPass = rsmIndirectHighResPass;
    rsmIndirectHighResPass ->addOutput(std::make_shared<Texture2D>("rsm_indirect_out_highres",
                                                Texture::RGB16F));
    rsmIndirectHighResPass->setCustomFragmentNameMapping("rsm_indirect_out_highres", "rsm_indirect_out");
    rsmIndirectHighResPass->setProperty("highres", true);

    _rsmIndirectHighResPlane = new RSMIndirectPlane();
    rsmIndirectHighResPass->addRenderer(_rsmIndirectHighResPlane);
    rsmIndirectHighResPass->setBlendFunc(GL_ONE, GL_ONE);

    auto rsmFinalPass = addPass();
    rsmFinalPass->addOutput(std::make_shared<Texture2D>("rsm_indirect_out", Texture::RGB16F));
    auto rsmFinalPlane = new PixelPlane();
    rsmFinalPlane->setProvider<RSMFinalProvider>();
    rsmFinalPass->addRenderer(rsmFinalPlane);
}

void RSMEvaluationBlock::setCamera(std::shared_ptr<Camera> cam)
{
    RenderBlock::setCamera(cam);
    _rsmIndirectPass.lock()->setCamera(cam);
    auto lowrescam = std::dynamic_pointer_cast<Camera>(cam->clone());
    int w = cam->getWidth() / 16;
    int h = cam->getHeight() / 16;
    lowrescam->setResolution(w, h);
    _rsmIndirectLowResPass.lock()->setCamera(lowrescam);
}

void RSMEvaluationBlock::setGeometry(std::shared_ptr<Group> grp)
{
    auto config = _config->getManager()->getConfig();
    if(!config.hasProperty("defaultLighting") ||
       !config["defaultLighting"].getData<bool>()) {
        _rsmIndirectHighResPlane->setLights(grp->getLights());
        _rsmIndirectLowResPlane->setLights(grp->getLights());
    }
    auto shadowPasses = _shadowBlock->getShadowPasses();
    _rsmIndirectHighResPlane->setShadowPasses(shadowPasses);
    _rsmIndirectLowResPlane->setShadowPasses(shadowPasses);

    if (grp->hasProperty("RSM:searchRadius")) {
        _rsmIndirectHighResPlane->setSearchRadius(grp->getProperty("RSM:searchRadius").getData<double>());
        _rsmIndirectLowResPlane->setSearchRadius(grp->getProperty("RSM:searchRadius").getData<double>());
    }
    if (grp->hasProperty("RSM:intensity")) {
        _rsmIndirectHighResPlane->setIntensity(grp->getProperty("RSM:intensity").getData<double>());
        _rsmIndirectLowResPlane->setIntensity(grp->getProperty("RSM:intensity").getData<double>());
    }
    if (grp->hasProperty("RSM:samples")) {
        _rsmIndirectHighResPlane->setSamples(grp->getProperty("RSM:samples").getData<int>());
        _rsmIndirectLowResPlane->setSamples(grp->getProperty("RSM:samples").getData<int>());
    }
    if(grp->hasProperty("RSM:enabled")) {
        setEnabled(grp->getProperty("RSM:enabled").getData<bool>());
    }
}

