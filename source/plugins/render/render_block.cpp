#include "glm/gtc/matrix_transform.hpp"
#include "primitive_renderer.h"
#include "polygon_renderer.h"
#include "renderpass.h"
#include "light_renderer.h"
#include "camera_renderer.h"
#include "coordsystem_renderer.h"
#include "empty_renderer.h"
#include "skeleton_renderer.h"

#include "render_setup.h"
#include "rendertree.h"
#include "benchmark.h"
#include "string"

#include "render_block.h"

using namespace MindTree;
using namespace GL;

RenderBlock::RenderBlock()
    : _config(nullptr)
{

}

void RenderBlock::setBenchmark(std::shared_ptr<Benchmark> benchmark)
{
    _benchmark = benchmark;
    if (!benchmark)
        return;

    std::string name_base = _benchmark->getName();
    for (int i = 0; i < _passes.size(); ++i) {
        auto pass = _passes[i];

        std::string name = name_base + std::to_string(i + 1);
        auto bench = std::make_shared<Benchmark>(name);
        pass->setBenchmark(bench);
        _benchmark->addBenchmark(bench);
    }
}

void RenderBlock::addOutput(Texture2D *output)
{
    _outputs.push_back(output);
}

std::vector<Texture2D*> RenderBlock::getOutputs() const
{
    return _outputs;
}

std::weak_ptr<Benchmark> RenderBlock::getBenchmark() const
{
    return _benchmark;
}

void RenderBlock::setEnabled(bool enable)
{
    for (auto &pass : _passes) {
        pass->setEnabled(enable);
    }
}

std::weak_ptr<Camera> RenderBlock::getCamera() const
{
    return _camera;
}

void RenderBlock::setCamera(std::shared_ptr<Camera> camera)
{
    _camera = camera;
    for (auto &pass : _passes)
        pass->setCamera(camera);
}

void RenderBlock::setProperty(std::string name, Property prop)
{
    for(auto &p : _passes)
        p->setProperty(name, prop);
}

void RenderBlock::setGeometry(std::shared_ptr<Group> grp)
{
    setRenderersFromGroup(grp);
}

void RenderBlock::setRenderersFromGroup(std::shared_ptr<Group> group)
{
    addRenderersFromGroup(group->getMembers());
}

void RenderBlock::addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group)
{
    for(const auto &transformable : group) {
        addRendererFromTransformable(transformable);
    }
}

void RenderBlock::addRendererFromTransformable(AbstractTransformablePtr transformable)
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
        case AbstractTransformable::JOINT:
            addRendererFromJoint(std::dynamic_pointer_cast<Joint>(transformable));
            break;
    }
    addRenderersFromGroup(transformable->getChildren());
}

void RenderBlock::addRendererFromObject(GeoObjectPtr obj)
{
}

void RenderBlock::addRendererFromLight(LightPtr obj)
{
}

void RenderBlock::addRendererFromCamera(CameraPtr obj)
{
}

void RenderBlock::addRendererFromEmpty(EmptyPtr obj)
{
}

void RenderBlock::addRendererFromJoint(JointPtr obj)
{
}

RenderPass* RenderBlock::addPass(const std::string &name)
{
    auto pass = std::make_unique<RenderPass>(name);
    auto pass_ptr = pass.get();
    _passes.push_back(pass_ptr);

    _config->getManager()->addPass(std::move(pass));

    return pass_ptr;
}


RenderPass* RenderBlock::addPassBefore(const RenderPass *hint, const std::string &name)
{
    auto pass = std::make_unique<RenderPass>(name);
    auto pass_ptr = pass.get();
    _passes.push_back(pass_ptr);

    _config->getManager()->insertPassBefore(hint, std::move(pass));

    return pass_ptr;
}

GeometryRenderBlock::GeometryRenderBlock(RenderPass *geopass)
    : _geometryPass(geopass)
{
}

void GeometryRenderBlock::setGeometry(std::shared_ptr<Group> grp)
{
    setRenderersFromGroup(grp);
}

void GeometryRenderBlock::setRenderersFromGroup(std::shared_ptr<Group> group)
{
    _geometryPass->clearRenderers();
    addRenderersFromGroup(group->getMembers());
    _geometryPass->clearUnusedShaderNodes();
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
    //auto coord = new CoordSystemRenderer();
    //coord->setTransformation(transformable->getWorldTransformation());
    //_geometryPass->addGeometryRenderer(coord);
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
        case AbstractTransformable::JOINT:
            addRendererFromJoint(std::dynamic_pointer_cast<Joint>(transformable));
            break;
    }
    addRenderersFromGroup(transformable->getChildren());
}

void GeometryRenderBlock::addRendererFromObject(GeoObjectPtr obj)
{
    auto data = obj->getData();
    switch(data->getType()){
        case ObjectData::MESH:
            if(obj->getData()->hasProperty("polygon")) {
                _geometryPass->addGeometryRenderer(new PolygonRenderer(obj));
                _geometryPass->addGeometryRenderer(new EdgeRenderer(obj));
            }
            _geometryPass->addGeometryRenderer(new PointRenderer(obj));
            break;
        case ObjectData::POINTCLOUD:
            _geometryPass->addGeometryRenderer(new PointRenderer(obj));
            break;
    }
}

void GeometryRenderBlock::addRendererFromLight(LightPtr obj)
{
    assert(obj);
    switch(obj->getLightType()) {
        case Light::POINT:
            _geometryPass->addGeometryRenderer(new PointLightRenderer(std::dynamic_pointer_cast<PointLight>(obj)));
            break;
        case Light::SPOT:
            _geometryPass->addGeometryRenderer(new SpotLightRenderer(std::dynamic_pointer_cast<SpotLight>(obj)));
            break;
        case Light::DISTANT:
            _geometryPass->addGeometryRenderer(new DistantLightRenderer(std::dynamic_pointer_cast<DistantLight>(obj)));
            break;
    }

}

void GeometryRenderBlock::addRendererFromCamera(CameraPtr obj)
{
    _geometryPass->addGeometryRenderer(new CameraRenderer(obj));
}

void GeometryRenderBlock::addRendererFromEmpty(EmptyPtr obj)
{
    _geometryPass->addGeometryRenderer(new EmptyRenderer(obj));
}

void GeometryRenderBlock::addRendererFromJoint(JointPtr obj)
{
    if(obj->getParent() && obj->getParent()->getType() == AbstractTransformable::JOINT) {
        return;
    }

    _geometryPass->addGeometryRenderer(new SkeletonRenderer(obj));
}

RenderPass* GeometryRenderBlock::getGeometryPass() const
{
    return _geometryPass;
}
