#include "random"
#include "glm/gtx/string_cast.hpp"
#include "render_setup.h"
#include "rendertree.h"
#include "data/benchmark.h"
#include "rsm_computation_plane.h"

using namespace MindTree;
using namespace GL;

RSMIndirectPlane::RSMIndirectPlane() :
    _intensity(1.f),
    _searchRadius(.5f),
    _numSamples(400),
    _samplesChanged(false)
{
    setFragmentShader("../plugins/render/defaultShaders/rsm_indirect_lighting.frag");
}

void RSMIndirectPlane::init(ShaderProgram* program)
{
    PixelPlane::init(program);
    initSamplingTexture();
}

void RSMIndirectPlane::initSamplingTexture()
{
    _samplingPattern = make_resource<Texture>(getResourceManager(),
                                              "samplingPattern",
                                              Texture::RG32F);

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

void RSMIndirectPlane::drawLight(const LightPtr &light, ShaderProgram* program)
{
    if(light->getLightType() != Light::SPOT)
        return;

    if(_samplesChanged)
        initSamplingTexture();

    program->setTexture(_samplingPattern.get());
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

RenderPass* RSMGenerationBlock::createShadowPass(SpotLightPtr spot)
{
    auto shadowPass = ShadowMappingRenderBlock::createShadowPass(spot);

    if(!shadowPass)
        return nullptr;

    if(_benchmark) {
        std::string name = _benchmark->getName();
        std::string spot_name = spot->getName();
        auto bench = std::make_shared<Benchmark>(name + "_" + spot_name);
        _benchmark->addBenchmark(bench);
        shadowPass->setBenchmark(bench);
    }

    auto pos = make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                        "shadow_position",
                                        Texture::RGBA16F);
    pos->setFilter(Texture::NEAREST);
    pos->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(std::move(pos));
    auto normal = make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                           "shadow_normal",
                                           Texture::RGBA16F);
    normal->setFilter(Texture::NEAREST);
    normal->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(std::move(normal));
    auto flux = make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                         "shadow_flux",
                                         Texture::RGBA16F);
    flux->setFilter(Texture::NEAREST);
    flux->setWrapMode(Texture::REPEAT);
    shadowPass->addOutput(std::move(flux));

    return shadowPass;
}

RSMEvaluationBlock::RSMEvaluationBlock(RSMGenerationBlock *shadowBlock) :
    _rsmIndirectHighResPlane(nullptr),
    _rsmIndirectLowResPlane(nullptr),
    _rsmIndirectPass(nullptr),
    _rsmIndirectLowResPass(nullptr),
    _rsmInterpolatePass(nullptr),
    _downSampling(2),
    _shadowBlock(shadowBlock)
{
}

void RSMEvaluationBlock::init()
{
    PropertyMap settings = {
        { "RSM:enable", false },
        { "RSM:searchRadius", 1.0 },
        { "RSM:intensity", 1.0 },
        { "RSM:samples", 128 },
        { "RSM:downsampling", 4 },
        { "RSM:lowresdistance", 0.1 },
        { "RSM:lowresangle", 0.5 },
    };
    //_config->addSettings("RSM Evaluation", settings);

    auto rsmIndirectLowResPass = addPass("rsm_lowres");
    _rsmIndirectLowResPass = rsmIndirectLowResPass;
    rsmIndirectLowResPass->addOutput(make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                              "rsm_indirect_out_lowres",
                                                              Texture::RGBA16F));
    rsmIndirectLowResPass->setCustomFragmentNameMapping("rsm_indirect_out_lowres", "rsm_indirect_out");

    _rsmIndirectLowResPlane = new RSMIndirectPlane();
    rsmIndirectLowResPass->addRenderer(_rsmIndirectLowResPlane);
    rsmIndirectLowResPass->setBlendFunc(GL_ONE, GL_ONE);
    rsmIndirectLowResPass->setBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

    _rsmInterpolatePass = addPass("rsm_interpolate");
    _rsmInterpolatePass->addOutput(make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                           "rsm_indirect_out_interpolated",
                                                           Texture::RGBA16F));
    _rsmInterpolatePass->addOutput(make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                           "rsm_indirect_interpolate_mask",
                                                           Texture::RGBA8));
    _rsmInterpolatePass->addOutput(make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                           "rsm_indirect_lowres_pixels",
                                                           Texture::RGBA8));
    _rsmInterpolatePass->setProperty("downsampling", _downSampling.load());
    auto rsmInterpolatePlane = new PixelPlane("../plugins/render/defaultShaders/rsm_interpolate.frag");
    _rsmInterpolatePass->addRenderer(rsmInterpolatePlane);

    _rsmIndirectPass = addPass("rsm_indirect");
    _rsmIndirectPass->addOutput(make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                         "rsm_indirect_out_highres",
                                                         Texture::RGBA16F));
    _rsmIndirectPass->setCustomFragmentNameMapping("rsm_indirect_out_highres", "rsm_indirect_out");
    _rsmIndirectPass->setProperty("highres", true);

    _rsmIndirectHighResPlane = new RSMIndirectPlane();
    _rsmIndirectPass->addRenderer(_rsmIndirectHighResPlane);
    _rsmIndirectPass->setBlendFunc(GL_ONE, GL_ONE);
    _rsmIndirectPass->setBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

    auto rsmFinalPass = addPass("rsm_final");
    rsmFinalPass->addOutput(make_resource<Texture2D>(_config->getTree()->getResourceManager(),
                                                     "rsm_indirect_out",
                                                     Texture::RGBA16F));
    auto rsmFinalPlane = new PixelPlane("../plugins/render/defaultShaders/rsm_final.frag");
    rsmFinalPass->addRenderer(rsmFinalPlane);

    setBenchmark(std::make_shared<Benchmark>("RSM Evaluation"));
    addOutput(rsmFinalPass->getOutputTextures()[0]);
}

void RSMEvaluationBlock::setCamera(std::shared_ptr<Camera> cam)
{
    RenderBlock::setCamera(cam);
    _rsmIndirectPass->setCamera(cam);
    auto lowrescam = std::dynamic_pointer_cast<Camera>(cam->clone());
    int downres = pow(2, _downSampling.load());
    int w = cam->getWidth() / downres;
    int h = cam->getHeight() / downres;
    lowrescam->setResolution(w, h);
    _rsmIndirectLowResPass->setCamera(lowrescam);
}

void RSMEvaluationBlock::setGeometry(std::shared_ptr<Group> grp)
{
    if(grp->hasProperty("RSM:enable")) {
        bool enable = grp->getProperty("RSM:enable").getData<bool>();
        setEnabled(enable);
        if(!enable)
            return;
    }

    auto config = _config->getTree()->getConfig();
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
        _rsmInterpolatePass->setProperty("downsampling", _downSampling.load());
        setCamera(getCamera().lock());
    }
    if (grp->hasProperty("RSM:lowresdistance")) {
        auto prop = grp->getProperty("RSM:lowresdistance");
        _rsmInterpolatePass->setProperty("distanceTolerance", prop);
    }
    if (grp->hasProperty("RSM:lowresangle")) {
        auto prop = grp->getProperty("RSM:lowresangle");
        _rsmInterpolatePass->setProperty("cosAngleTolerance", prop);
    }
}

