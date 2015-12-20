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

    auto depth = std::make_shared<Texture2D>("depth",
                                             Texture::DEPTH);
    geopass->setDepthOutput(depth);
    geopass->addOutput(std::make_shared<Texture2D>("outdiffusecolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outcolor"));
    geopass->addOutput(std::make_shared<Texture2D>("outdiffuseintensity"));
    geopass->addOutput(std::make_shared<Texture2D>("outspecintensity"));
    auto normal = std::make_shared<Texture2D>("outnormal",
                                              Texture::RGBA16F);
    normal->generateMipmaps();
    geopass->addOutput(normal);
    auto position = std::make_shared<Texture2D>("outposition",
                                                Texture::RGBA16F);
    position->generateMipmaps();
    geopass->addOutput(position);
    geopass->addOutput(std::make_shared<Texture2D>("worldposition",
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
    auto gbufferShader = std::make_shared<ShaderProgram>();
    gbufferShader
        ->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert",
                            ShaderProgram::VERTEX);
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
    if(name == "defaultLighting") {
        _geometryPass.lock()->setProperty(name, prop);
    }
}

