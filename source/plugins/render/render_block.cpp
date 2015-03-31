#include "glm/gtc/matrix_transform.hpp"
#include "primitive_renderer.h"
#include "polygon_renderer.h"
#include "renderpass.h"
#include "light_renderer.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "render_setup.h"
#include "rendertree.h"
#include "render_block.h"

using namespace MindTree;
using namespace GL;

RenderBlock::RenderBlock()
    : _config(nullptr)
{

}

void RenderBlock::setCamera(std::shared_ptr<Camera> camera)
{
    for (auto pass : _passes)
        pass.lock()->setCamera(camera);
}

void RenderBlock::setGeometry(std::shared_ptr<Group> grp)
{
}

std::shared_ptr<RenderPass> RenderBlock::addPass()
{
    auto pass = std::make_shared<RenderPass>();
    _passes.push_back(pass);

    _config->getManager()->addPass(pass);

    return pass;
}

GeometryRenderBlock::GeometryRenderBlock(std::weak_ptr<RenderPass> geopass)
    : _geometryPass(geopass)
{
}

void GeometryRenderBlock::setGeometry(std::shared_ptr<Group> grp)
{
    setRenderersFromGroup(grp);
}

void GeometryRenderBlock::setRenderersFromGroup(std::shared_ptr<Group> group)
{
    _geometryPass.lock()->clearRenderers();
    addRenderersFromGroup(group->getMembers());
    _geometryPass.lock()->clearUnusedShaderNodes();
}

void GeometryRenderBlock::addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group)
{
    for(const auto &transformable : group) {
        addRendererFromTransformable(transformable);
    }
}

void GeometryRenderBlock::addRendererFromTransformable(AbstractTransformablePtr transformable)
{
    assert(transformable);
    switch(transformable->getType()) {
        case AbstractTransformable::GEO:
            addRendererFromObject(std::dynamic_pointer_cast<GeoObject>(transformable));
            break;
        case AbstractTransformable::LIGHT:
            addRendererFromLight(std::dynamic_pointer_cast<Light>(transformable));
            break;
        case AbstractTransformable::CAMERA:
            addRendererFromCamera(std::dynamic_pointer_cast<Camera>(transformable));
            break;
        case AbstractTransformable::EMPTY:
            addRendererFromEmpty(std::dynamic_pointer_cast<Empty>(transformable));
            break;
    }
    addRenderersFromGroup(transformable->getChildren());
}

void GeometryRenderBlock::addRendererFromObject(GeoObjectPtr obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            _geometryPass.lock()->addGeometryRenderer(new PolygonRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new EdgeRenderer(obj));
            _geometryPass.lock()->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void GeometryRenderBlock::addRendererFromLight(LightPtr obj)
{
    assert(obj);
    switch(obj->getLightType()) {
        case Light::POINT:
            _geometryPass.lock()->addGeometryRenderer(new PointLightRenderer(std::dynamic_pointer_cast<PointLight>(obj)));
            break;
        case Light::SPOT:
            _geometryPass.lock()->addGeometryRenderer(new SpotLightRenderer(std::dynamic_pointer_cast<SpotLight>(obj)));
            break;
        case Light::DISTANT:
            _geometryPass.lock()->addGeometryRenderer(new DistantLightRenderer(std::dynamic_pointer_cast<DistantLight>(obj)));
            break;
    }

}

void GeometryRenderBlock::addRendererFromCamera(CameraPtr obj)
{
    _geometryPass.lock()->addGeometryRenderer(new CameraRenderer(obj));
}

void GeometryRenderBlock::addRendererFromEmpty(EmptyPtr obj)
{
    _geometryPass.lock()->addGeometryRenderer(new EmptyRenderer(obj));
}

std::weak_ptr<RenderPass> GeometryRenderBlock::getGeometryPass() const
{
    return _geometryPass;
}
