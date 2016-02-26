#include "GL/glew.h"
#include "glm/gtc/matrix_transform.hpp"

#include "glwrapper.h"
#include "rendertree.h"
#include "data/debuglog.h"

#include "polygon_renderer.h"

using namespace MindTree::GL;

PolygonRenderer::PolygonRenderer(std::shared_ptr<GeoObject> o)
    : GeoObjectRenderer(o),
      _triangleCount(0)
{
}

PolygonRenderer::~PolygonRenderer()
{
}

template<>
const std::string ShaderFiles<PolygonRenderer>::
vertexShader = "../plugins/render/defaultShaders/polygons.vert";
template<>
const std::string ShaderFiles<PolygonRenderer>::
fragmentShader = "../plugins/render/defaultShaders/polygons.frag";

ShaderProgram* PolygonRenderer::getProgram()
{
    return getResourceManager()->shaderManager()->getProgram<PolygonRenderer>();
}

std::vector<uint> PolygonRenderer::triangulate()
{
    auto data = obj->getData();
    auto polygons = data->getProperty("polygon").getData<PolygonListPtr>();
    std::vector<uint> triangles;
    for(Polygon &poly : *polygons) {
        uint first = poly.verts()[0];
        for(size_t i = 1; i < poly.verts().size() - 1; ++i) {
            triangles.push_back(first);
            triangles.push_back(i);
            triangles.push_back(i+1);
        }
    }
    _triangleCount = triangles.size();
    return triangles;
}

void PolygonRenderer::initCustom()
{
    auto data = obj->getData();
    _triangulatedIBO = make_resource<IBO>(getResourceManager());
    _triangulatedIBO->bind();
    auto triangles = triangulate();
    _triangulatedIBO->data(triangles);
    if (std::static_pointer_cast<MeshData>(data)->hasProperty("polygon_color")) {
        auto colProp = std::static_pointer_cast<MeshData>(data)->getProperty("polygon_color");
        auto colors = colProp.getData<std::vector<glm::vec4>>();

        _polyColors = make_resource<Texture>(getResourceManager(), "polygon_colors", Texture::RGBA8);
        _polyColors->setWidth(colors.size());
        _polyColors->init(colors);
    }
}

void PolygonRenderer::draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    if(!config.drawPolygons()) return;
    GeoObjectRenderer::draw(camera, config, program);

    auto data = obj->getData();
    UniformStateManager manager(program);
    manager.addState("flatShading", (int)config.flatShading());

    if(_polyColors) program->setTexture(_polyColors.get());

    if(obj->getMaterial()) {
        manager.setFromPropertyMap(obj->getMaterial()->getProperties());
    }

    //program->setTexture(_polyColorTexture);
    glPolygonOffset(1.0, 1.0);
    MTGLERROR;
    glDrawElements(GL_TRIANGLES, //Primitive type
                   _triangleCount,
                   GL_UNSIGNED_INT, //index datatype
                   nullptr); //primitive count
    MTGLERROR;
}

EdgeRenderer::EdgeRenderer(std::shared_ptr<GeoObject> o)
    : GeoObjectRenderer(o)
{
}

EdgeRenderer::~EdgeRenderer()
{
}

template<>
const std::string ShaderFiles<EdgeRenderer>::
vertexShader = "../plugins/render/defaultShaders/edges.vert";
template<>
const std::string ShaderFiles<EdgeRenderer>::
fragmentShader = "../plugins/render/defaultShaders/edges.frag";

ShaderProgram* EdgeRenderer::getProgram()
{
    return getResourceManager()->shaderManager()->getProgram<EdgeRenderer>();
}

void EdgeRenderer::initCustom()
{
    auto data = obj->getData();
    auto ibo = getResourceManager()->geometryCache()->getIBO(data.get());
    ibo->bind();
    ibo->data(data->getProperty("polygon").getData<PolygonListPtr>());
}

void EdgeRenderer::draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    if(!config.drawEdges()) return;
    GeoObjectRenderer::draw(camera, config, program);

    float lineWidth = 1.5;

    if (obj->hasProperty("display.lineWidth"))
        lineWidth =  obj->getProperty("display.lineWidth").getData<double>();

    auto data = obj->getData();
    auto ibo = getResourceManager()->geometryCache()->getIBO(data.get());

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

PointRenderer::PointRenderer(std::shared_ptr<GeoObject> o)
    : GeoObjectRenderer(o)
{
}

PointRenderer::~PointRenderer()
{
}

template<>
const std::string ShaderFiles<PointRenderer>::
vertexShader = "../plugins/render/defaultShaders/points.vert";
template<>
const std::string ShaderFiles<PointRenderer>::
fragmentShader = "../plugins/render/defaultShaders/points.frag";

ShaderProgram* PointRenderer::getProgram()
{
    return getResourceManager()->shaderManager()->getProgram<PointRenderer>();
}

void PointRenderer::draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    if(!config.drawPoints()) return;
    GeoObjectRenderer::draw(camera, config, program);

    auto mesh = std::static_pointer_cast<MeshData>(obj->getData());
    auto verts = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    glDrawArrays(GL_POINTS, 0, verts->size());
    MTGLERROR;
}

