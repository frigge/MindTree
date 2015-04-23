#include "random"
#include "glm/gtx/string_cast.hpp"
#include "render_setup.h"
#include "rendertree.h"
#include "benchmark.h"
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
    auto shadowBench = std::make_shared<Benchmark>("RSM Generation");
    setBenchmark(shadowBench);
}

std::weak_ptr<RenderPass> RSMGenerationBlock::createShadowPass(SpotLightPtr spot)
{
    auto shadowPass = ShadowMappingRenderBlock::createShadowPass(spot).lock();

    if(!shadowPass)
        return std::weak_ptr<RenderPass>();

    if(_benchmark) {
        std::string name = _benchmark->getName();
        std::string spot_name = spot->getName();
        auto bench = std::make_shared<Benchmark>(name + "_" + spot_name);
        _benchmark->addBenchmark(bench);
        shadowPass->setBenchmark(bench);
    }

    auto pos = std::make_shared<Texture2D>("shadow_position",
                                                             Texture::RGBA16F);
    pos->setFilter(Texture::NEAREST);
    pos->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(pos);
    auto normal = std::make_shared<Texture2D>("shadow_normal",
                                                             Texture::RGBA16F);
    normal->setFilter(Texture::NEAREST);
    normal->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(normal);
    auto flux = std::make_shared<Texture2D>("shadow_flux", Texture::RGBA16F);
    flux->setFilter(Texture::NEAREST);
    shadowPass->addOutput(flux);
    flux->setWrapMode(Texture::REPEAT);

    return shadowPass;
}

RSMEvaluationBlock::RSMEvaluationBlock(RSMGenerationBlock *shadowBlock) :
    _shadowBlock(shadowBlock), _downSampling(2)
{
}

void RSMEvaluationBlock::init()
{
    auto rsmIndirectLowResPass = addPass();
    _rsmIndirectLowResPass = rsmIndirectLowResPass;
    rsmIndirectLowResPass->addOutput(std::make_shared<Texture2D>("rsm_indirect_out_lowres",
                                                Texture::RGB16F));
    rsmIndirectLowResPass->setCustomFragmentNameMapping("rsm_indirect_out_lowres", "rsm_indirect_out");

    _rsmIndirectLowResPlane = new RSMIndirectPlane();
    rsmIndirectLowResPass->addRenderer(_rsmIndirectLowResPlane);
    rsmIndirectLowResPass->setBlendFunc(GL_ONE, GL_ONE);

    _rsmInterpolatePass = addPass();
    auto rsmInterpolatePass = _rsmInterpolatePass.lock();
    rsmInterpolatePass->addOutput(std::make_shared<Texture2D>("rsm_indirect_out_interpolated", Texture::RGBA16F));
    rsmInterpolatePass->addOutput(std::make_shared<Texture2D>("rsm_indirect_interpolate_mask", Texture::RGBA8));
    rsmInterpolatePass->addOutput(std::make_shared<Texture2D>("rsm_indirect_lowres_pixels", Texture::RGBA8));
    rsmInterpolatePass->setProperty("downsampling", _downSampling.load());
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

    setBenchmark(std::make_shared<Benchmark>("RSM Evaluation"));
}

void RSMEvaluationBlock::setCamera(std::shared_ptr<Camera> cam)
{
    RenderBlock::setCamera(cam);
    _rsmIndirectPass.lock()->setCamera(cam);
    auto lowrescam = std::dynamic_pointer_cast<Camera>(cam->clone());
    int downres = pow(2, _downSampling.load());
    int w = cam->getWidth() / downres;
    int h = cam->getHeight() / downres;
    lowrescam->setResolution(w, h);
    _rsmIndirectLowResPass.lock()->setCamera(lowrescam);
}

void RSMEvaluationBlock::setGeometry(std::shared_ptr<Group> grp)
{
    if(grp->hasProperty("RSM:enable")) {
        bool enable = grp->getProperty("RSM:enable").getData<bool>();
        setEnabled(enable);
        if(!enable)
            return;
    }

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
    if (grp->hasProperty("RSM:downsampling")) {
        _downSampling = grp->getProperty("RSM:downsampling").getData<int>();
        _rsmInterpolatePass.lock()->setProperty("downsampling", _downSampling.load());
        setCamera(getCamera().lock());
    }
    if (auto prop = grp->getProperty("RSM:lowresdistance")) {
        _rsmInterpolatePass.lock()->setProperty("distanceTolerance", prop);
    }
    if (auto prop = grp->getProperty("RSM:lowresangle")) {
        _rsmInterpolatePass.lock()->setProperty("cosAngleTolerance", prop);
    }
}

