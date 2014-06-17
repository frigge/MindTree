#include "glwrapper.h"
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

void GeoObjectRenderer::initVAO()
{
    _vao = Context::getCurrent()->getManager()->getVAO(obj->getData());
}

void GeoObjectRenderer::init()    
{
    auto data = obj->getData();
    auto propmap = data->getProperties();
    auto prog = getProgram();
    for(auto propPair : propmap){
        bool has_attr = prog->hasAttribute(propPair.first);
        if(has_attr) {
            Context::getSharedContext()->getManager()->uploadData(data, propPair.first);
            auto vbo = Context::getSharedContext()->getManager()->getVBO(data, propPair.first);
            prog->bindAttributeLocation(vbo->getIndex(), propPair.first);
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
    auto propmap = obj->getProperties();
    for(const auto &p : propmap) {
        std::string str = p.first;
        std::string sub = str.substr(0, str.find("."));
        if(sub == "display") {
            if(p.second.getType() == "FLOAT")
                program->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 (float)p.second.getData<double>());

            else if(p.second.getType() == "INTEGER")
                program->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 p.second.getData<int>());

            else if(p.second.getType() == "BOOLEAN")
                program->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 p.second.getData<bool>());

            else if(p.second.getType() == "COLOR") {
                std::string temp = str.substr(str.find(".")+1, str.length());
                program->setUniform(temp, 
                                 p.second.getData<glm::vec4>());
            }

            else if(p.second.getType() == "VECTOR3D")
                program->setUniform(str.substr(str.find(".")+1, str.length()), 
                                 p.second.getData<glm::vec3>());
        }
    }
}

void GeoObjectRenderer::setUniforms()
{
}
