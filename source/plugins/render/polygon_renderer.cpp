#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"

#include "glwrapper.h"
#include "rendermanager.h"
#include "data/debuglog.h"

#include "polygon_renderer.h"

using namespace MindTree::GL;

std::weak_ptr<ShaderProgram> PolygonRenderer::_defaultProgram;

PolygonRenderer::PolygonRenderer(std::shared_ptr<GeoObject> o)
    : GeoObjectRenderer(o)
{
}

PolygonRenderer::~PolygonRenderer()
{
}

std::shared_ptr<ShaderProgram> PolygonRenderer::getProgram()
{
    std::shared_ptr<ShaderProgram> prog;
    if(_defaultProgram.expired()) {
        prog = std::make_shared<ShaderProgram>();
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/polygons.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/polygons.frag", 
                                ShaderProgram::FRAGMENT);
        _defaultProgram = prog;
    }

    return _defaultProgram.lock();
}

void PolygonRenderer::initCustom()
{
    auto data = obj->getData();
    auto ibo = RenderManager::getResourceManager()->getIBO(data);
    ibo->bind();
    ibo->data(data->getProperty("polygon").getData<PolygonListPtr>());
}

void PolygonRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    if(!config.drawPolygons()) return;
    GeoObjectRenderer::draw(camera, config, program);

    auto data = obj->getData();
    UniformStateManager manager(program);
    manager.addState("flatShading", (int)config.flatShading());
    if(obj->getMaterial()) {
        manager.setFromPropertyMap(obj->getMaterial()->getProperties());
    }

    auto ibo = RenderManager::getResourceManager()->getIBO(data);

    auto polysizes = ibo->getSizes();
    auto indexOffsets = ibo->getOffsets();

    glPolygonOffset(1.0, 1.0);

    glMultiDrawElements(GL_TRIANGLE_FAN, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        //(const GLvoid**)&polyindices[0],
                        reinterpret_cast<const GLvoid**>(&indexOffsets[0]),
                        polysizes.size()); //primitive count
    MTGLERROR;
}

std::weak_ptr<ShaderProgram> EdgeRenderer::_defaultProgram;

EdgeRenderer::EdgeRenderer(std::shared_ptr<GeoObject> o)
    : GeoObjectRenderer(o)
{
}

EdgeRenderer::~EdgeRenderer()
{
}

std::shared_ptr<ShaderProgram> EdgeRenderer::getProgram()
{
    std::shared_ptr<ShaderProgram> prog;
    if(_defaultProgram.expired()) {
        prog = std::make_shared<ShaderProgram>();
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/edges.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/edges.frag", 
                                ShaderProgram::FRAGMENT);
        _defaultProgram = prog;
    }

    return _defaultProgram.lock();
}

void EdgeRenderer::initCustom()
{
    auto data = obj->getData();
    auto ibo = RenderManager::getResourceManager()->getIBO(data);
    ibo->bind();
    ibo->data(data->getProperty("polygon").getData<PolygonListPtr>());
}

void EdgeRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    if(!config.drawEdges()) return;
    GeoObjectRenderer::draw(camera, config, program);

    float lineWidth = 1.5;

    if (obj->hasProperty("display.lineWidth"))
        lineWidth =  obj->getProperty("display.lineWidth").getData<double>();

    auto data = obj->getData();
    auto ibo = RenderManager::getResourceManager()->getIBO(data);

    auto polysizes = ibo->getSizes();
    auto indexOffsets = ibo->getOffsets();
    glEnable(GL_LINE_SMOOTH);

    glLineWidth(lineWidth);
    glMultiDrawElements(GL_LINE_LOOP, //Primitive type
                        (const GLsizei*)&polysizes[0], //polygon sizes
                        GL_UNSIGNED_INT, //index datatype
                        (const GLvoid**)&indexOffsets[0],
                        polysizes.size()); //primitive count
    MTGLERROR;
    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
}

std::weak_ptr<ShaderProgram> PointRenderer::_defaultProgram;

PointRenderer::PointRenderer(std::shared_ptr<GeoObject> o)
    : GeoObjectRenderer(o)
{
}

PointRenderer::~PointRenderer()
{
}

std::shared_ptr<ShaderProgram> PointRenderer::getProgram()
{
    std::shared_ptr<ShaderProgram> prog;
    if(_defaultProgram.expired()) {
        prog = std::make_shared<ShaderProgram>();
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/points.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/points.frag", 
                                ShaderProgram::FRAGMENT);
        _defaultProgram = prog;
    }
    return _defaultProgram.lock();
}

void PointRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    if(!config.drawPoints()) return;
    GeoObjectRenderer::draw(camera, config, program);

    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto verts = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    glDrawArrays(GL_POINTS, 0, verts->size());
    MTGLERROR;
}

