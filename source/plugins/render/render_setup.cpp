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
#include "render_block.h"
#include "benchmark.h"

#include "render_setup.h"

using namespace MindTree;
using namespace GL;

RenderConfigurator::RenderConfigurator(QGLContext *context, CameraPtr camera) :
    _rendertree(new RenderTree(context)),
    _camera(camera),
    _vertexCount(0),
    _polyCount(0)
{
    auto geometryPass = std::make_shared<RenderPass>();
    _geometryPass = geometryPass;
    _rendertree->addPass(geometryPass);
    _geometryPass.lock()->setCamera(camera);

    _grid = new GL::GridRenderer(100, 100, 100, 100);
    auto trans = glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0));

    _grid->setTransformation(trans);
    _grid->setBorderColor(glm::vec4(.5, .5, .5, .5));
    _grid->setAlternatingColor(glm::vec4(.8, .8, .8, .8));
    _grid->setBorderWidth(2.);

    _geometryPass.lock()->addRenderer(_grid);
}

void RenderConfigurator::startRendering()
{
    MindTree::GL::RenderThread::addManager(_rendertree.get());
}

void RenderConfigurator::stopRendering()
{
    MindTree::GL::RenderThread::removeManager(_rendertree.get());
}

std::weak_ptr<RenderPass> RenderConfigurator::getGeometryPass() const
{
    return _geometryPass;
}

void RenderConfigurator::addRenderBlock(std::shared_ptr<RenderBlock> block)
{
    block->_config = this;
    block->init();
    auto bench = _rendertree->getBenchmark();
    if(!bench.expired()) {
        bench.lock()->addBenchmark(block->getBenchmark().lock());
    }

    block->setCamera(_camera);
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

void RenderConfigurator::setProperty(std::string name, Property prop)
{
    Object::setProperty(name, prop);
    for(auto block : _renderBlocks) {
        block->setProperty(name, prop);
    }
}

int RenderConfigurator::getPolygonCount() const
{
    return _polyCount;
}

int RenderConfigurator::getVertexCount() const
{
    return _vertexCount;
}

void RenderConfigurator::setGeometry(std::shared_ptr<Group> grp)
{
    _vertexCount = grp->getVertexCount();
    _polyCount = grp->getPolygonCount();
    for(auto block : _renderBlocks) {
        block->setGeometry(grp);
    }
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
