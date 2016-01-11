#include "glwrapper.h"
#include "rendertree.h"
#include "geoobject_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

GeoObjectRenderer::GeoObjectRenderer(std::shared_ptr<GeoObject> o)
    : obj(o)
{
    setTransformation(obj->getWorldTransformation());
}

GeoObjectRenderer::~GeoObjectRenderer()
{
}

void GeoObjectRenderer::init(ShaderProgram* prog)    
{
    auto data = obj->getData();
    auto propmap = data->getProperties();
    for(auto propPair : propmap){
        bool has_attr = prog->hasAttribute(propPair.first);
        if(has_attr) {
            RenderTree::getResourceManager()->uploadData(data, propPair.first);
            auto vbo = RenderTree::getResourceManager()->getVBO(data, propPair.first);
            prog->bindAttributeLocation(vbo);
        }
    }
    
    initCustom();
}

void GeoObjectRenderer::initCustom()
{
}

void GeoObjectRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    //setting uniforms
    UniformStateManager uniformStates(program);
    uniformStates.setFromPropertyMap(obj->getProperties());
}

void GeoObjectRenderer::setUniforms()
{
}
