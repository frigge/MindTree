#include "render.h"
#include "renderpass.h"
#include "primitive_renderer.h"
#include "rendertree.h"
#include "../3dwidgets/widgets.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "data/debuglog.h"
#include "polygon_renderer.h"
#include "light_renderer.h"
#include "camera_renderer.h"
#include "empty_renderer.h"
#include "shader_render_node.h"

#include "render_setup.h"

using namespace MindTree;
using namespace GL;

RenderConfigurator::RenderConfigurator(QGLContext *context, CameraPtr camera) :
    _rendertree(new RenderTree(context)),
    _camera(camera)
{

}

void RenderConfigurator::startRendering()
{
    MindTree::GL::RenderThread::addManager(_rendertree.get());
}

void RenderConfigurator::stopRendering()
{
    MindTree::GL::RenderThread::removeManager(_rendertree.get());
}

void RenderConfigurator::addRenderBlock(std::shared_ptr<RenderBlock> block)
{
    _renderBlocks.push_back(block);
}

glm::vec4 RenderConfigurator::getPosition(glm::vec2 pixel) const
{
    return glm::vec4(0);
}

RenderTree* RenderConfigurator::getManager()
{
    return _rendertree.get();
}

void RenderConfigurator::setOverrideOutput(std::string output)
{
}

void RenderConfigurator::clearOverrideOutput()
{
}

void RenderConfigurator::setRenderersFromGroup(std::shared_ptr<Group> group)
{
    _geometryPass.lock()->clearRenderers();
    addRenderersFromGroup(group->getMembers());
    _geometryPass.lock()->clearUnusedShaderNodes();
}

void RenderConfigurator::addRenderersFromGroup(std::vector<std::shared_ptr<AbstractTransformable>> group)
{
    for(const auto &transformable : group) {
        addRendererFromTransformable(transformable);
    }
}

void RenderConfigurator::addRendererFromTransformable(AbstractTransformablePtr transformable)
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

void RenderConfigurator::addRendererFromObject(GeoObjectPtr obj)
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

void RenderConfigurator::addRendererFromLight(LightPtr obj)
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

void RenderConfigurator::addRendererFromCamera(CameraPtr obj)
{
    _geometryPass.lock()->addGeometryRenderer(new CameraRenderer(obj));
}

void RenderConfigurator::addRendererFromEmpty(EmptyPtr obj)
{
    _geometryPass.lock()->addGeometryRenderer(new EmptyRenderer(obj));
}

void RenderConfigurator::setGeometry(std::shared_ptr<Group> grp)
{
    for(auto block : _renderBlocks) {
        block->setGeometry(grp);
    }
    setRenderersFromGroup(grp);
}

void RenderConfigurator::setCamera(std::shared_ptr<Camera> camera)
{
    for(auto block : _renderBlocks) {
        block->setCamera(camera);
    }
    _camera = camera;
    _rendertree->setDirty();
}

std::shared_ptr<Camera> RenderConfigurator::getCamera() const
{
    return _camera;
}

