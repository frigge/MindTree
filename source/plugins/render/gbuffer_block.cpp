#include "benchmark.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "polygon_renderer.h"
#include "render.h"
#include "renderpass.h"
#include "rendertree.h"
#include "shader_render_node.h"

#include "gbuffer_block.h"

using namespace MindTree;
using namespace MindTree::GL;

GBufferRenderBlock::GBufferRenderBlock(std::weak_ptr<RenderPass> geopass)
    : GeometryRenderBlock(geopass)
{
    auto gbBench = std::make_shared<Benchmark>("GBuffer Creation");
    geopass.lock()->setBenchmark(gbBench);
    setBenchmark(std::make_shared<Benchmark>("GBuffer"));
    getBenchmark().lock()->addBenchmark(gbBench);
}

GBufferRenderBlock::~GBufferRenderBlock()
{
}

void GBufferRenderBlock::init()
{
    auto geopass = _geometryPass.lock();

    geopass->setEnableBlending(false);

    auto depth = make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                          "depth",
                                          Texture::DEPTH);
    geopass->setDepthOutput(std::move(depth));
    geopass->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                "outdiffusecolor"));
    geopass->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                "outcolor"));
    geopass->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                "outdiffuseintensity"));
    geopass->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                "outspecintensity"));
    auto normalptr = make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                           "outnormal",
                                           Texture::RGBA16F);
    auto *normal = normalptr.get();
    normal->generateMipmaps();
    geopass->addOutput(std::move(normalptr));
    auto positionptr = make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                             "outposition",
                                             Texture::RGBA16F);
    auto *position = positionptr.get();
    position->generateMipmaps();
    geopass->addOutput(std::move(positionptr));
    geopass->addOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                "worldposition",
                                                Texture::RGBA16F));

    geopass->addPostRenderCallback([normal, position] (RenderPass *) {
                                       normal->generateMipmaps();
                                       position->generateMipmaps();
                                   });

    //register the outputs on the block
    //used for compositing later on
    for(auto tx : geopass->getOutputTextures())
        addOutput(tx);
    setupGBuffer();
}

void GBufferRenderBlock::setupGBuffer()
{
    auto gbufferShader = _config->
        getManager()->getResourceManager()->shaderManager()->getProgram<GBufferRenderBlock>();
    gbufferShader
        ->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert",
                            ShaderProgram::VERTEX);
    //    gbufferShader
    //            ->addShaderFromFile("../plugins/render/defaultShaders/polygons.geo",
    //                                ShaderProgram::GEOMETRY);
    gbufferShader
        ->addShaderFromFile("../plugins/render/defaultShaders/gbuffer.frag",
                            ShaderProgram::FRAGMENT);
    auto gbufferNode = std::make_shared<ShaderRenderNode>(gbufferShader);
    _gbufferNode = gbufferNode;

    _geometryPass.lock()->addGeometryShaderNode(gbufferNode);
}

void GBufferRenderBlock::addRendererFromObject(std::shared_ptr<GeoObject> obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            if(obj->getData()->hasProperty("polygon")) {
                _gbufferNode->addRenderer(new PolygonRenderer(obj));
                _geometryPass.lock()->addGeometryRenderer(new EdgeRenderer(obj));
            }
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
        case ObjectData::POINTCLOUD:
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void GBufferRenderBlock::setProperty(std::string name, Property prop)
{
    _geometryPass.lock()->setProperty(name, prop);
}

