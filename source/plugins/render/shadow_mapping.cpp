#include "glwrapper.h"
#include "render_setup.h"
#include "rendertree.h"
#include "shader_render_node.h"
#include "polygon_renderer.h"
#include "renderpass.h"
#include "../datatypes/Object/lights.h"

#include "shadow_mapping.h"

using namespace MindTree;
using namespace MindTree::GL;

ShadowMappingRenderBlock::ShadowMappingRenderBlock()
{
}

void ShadowMappingRenderBlock::init()
{
    auto shadowShader = _config->
        getManager()->getResourceManager()->shaderManager()->getProgram<ShadowMappingRenderBlock>();
    _shadowNode = std::make_shared<ShaderRenderNode>(shadowShader);
    _shadowNode->setResourceManager(_config->getManager()->getResourceManager());

    shadowShader
        ->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert",
                            ShaderProgram::VERTEX);
    shadowShader
        ->addShaderFromFile("../plugins/render/defaultShaders/shadow.frag",
                            ShaderProgram::FRAGMENT);
}

void ShadowMappingRenderBlock::addRendererFromLight(LightPtr obj)
{
    switch(obj->getLightType()) {
        case Light::POINT:
        case Light::DISTANT:
            break;
        case Light::SPOT:
           createShadowPass(std::dynamic_pointer_cast<SpotLight>(obj));
    }
}

void ShadowMappingRenderBlock::setGeometry(std::shared_ptr<Group> grp)
{
    //clear shadow passes
    for(auto p : _shadowPasses) {
        _config->getManager()->removePass(p.second);
    }
    _shadowNode->clear();
    _shadowPasses.clear();

    setRenderersFromGroup(grp);
}

void ShadowMappingRenderBlock::addRendererFromObject(std::shared_ptr<GeoObject> obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            _shadowNode->addRenderer(new PolygonRenderer(obj));
            break;
        case ObjectData::POINTCLOUD:
            break;
    }
}

std::unordered_map<std::shared_ptr<Light>, std::weak_ptr<RenderPass>> ShadowMappingRenderBlock::getShadowPasses() const
{
    return _shadowPasses;
}

std::weak_ptr<RenderPass> ShadowMappingRenderBlock::createShadowPass(SpotLightPtr spot)
{
    Light::ShadowInfo info = spot->getShadowInfo();
    if(!info._enabled) return std::weak_ptr<RenderPass>();

    auto shadowPass = std::make_shared<RenderPass>();

    _shadowPasses[spot] = shadowPass;

    auto camera = std::make_shared<Camera>();
    camera->setResolution(info._size.x, info._size.y);
    camera->setTransformation(spot->getWorldTransformation());
    camera->setFov(spot->getConeAngle() * 2);
    camera->setNear(info._near);
    camera->setFar(info._far);
    shadowPass->setCamera(camera);
    shadowPass
        ->setDepthOutput(make_resource<Texture2D>(_config->getManager()->getResourceManager(),
                                                  "shadow",
                                                  Texture::DEPTH32F));
    shadowPass->addGeometryShaderNode(_shadowNode);
    shadowPass->setClearDepth(1.);
    static const float PI = 3.14159265359;
    shadowPass->setProperty("coneangle", spot->getConeAngle() * PI /180);
    shadowPass->setProperty("intensity", spot->getIntensity());

    _config->getManager()->insertPassAfter(_config->getGeometryPass(), shadowPass);

    return shadowPass;
}

