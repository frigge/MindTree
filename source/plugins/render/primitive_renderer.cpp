#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "GL/glew.h"
#include "renderpass.h"
#include "rendertree.h"
#include "primitive_renderer.h"

#define PI 3.14159265359

using namespace MindTree;
using namespace MindTree::GL;

ShapeRendererGroup::ShapeRendererGroup(ShapeRendererGroup *parent)
    : _fillColor(glm::vec4(1)),
    _borderColor(glm::vec4(1)),
    _borderWidth(1.),
    _fixedScreenSize(false),
    _screenOriented(false)
{
    if(parent) setParentPrimitive(parent);
}

void ShapeRendererGroup::setParentPrimitive(ShapeRendererGroup *renderer)
{
    setParent(renderer);
    renderer->setChildPrimitive(this);
}

void ShapeRendererGroup::setChildPrimitive(ShapeRendererGroup *renderer)
{
    addChild(renderer);
    _childPrimitives.push_back(renderer);
}

void ShapeRendererGroup::setStaticTransformation(glm::mat4 trans)
{
    _staticTransformation = trans;
}

void ShapeRendererGroup::staticTransform(glm::mat4 trans)
{
    _staticTransformation = trans * _staticTransformation;
}

glm::mat4 ShapeRendererGroup::getStaticWorldTransformation() const
{
    auto parent = dynamic_cast<const ShapeRendererGroup*>(getParent());
    if (parent) {
        return parent->getStaticWorldTransformation() * _staticTransformation;
    }
    return _staticTransformation;
}

void ShapeRendererGroup::init(ShaderProgram* prog)
{
    /* nothing to do here */
}

void ShapeRendererGroup::draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
}

void ShapeRendererGroup::setFillColor(glm::vec4 color)
{
    std::lock_guard<std::mutex> lock(_fillColorLock);
    _fillColor = color;
    for(auto child : _childPrimitives)
        child->setFillColor(color);
}

void ShapeRendererGroup::setBorderColor(glm::vec4 color)
{
    std::lock_guard<std::mutex> lock(_borderColorLock);
    _borderColor = color;
    for(auto child : _childPrimitives)
        child->setBorderColor(color);
}

glm::vec4 ShapeRendererGroup::getBorderColor() const
{
    std::lock_guard<std::mutex> lock(_borderColorLock);
    return _borderColor;
}

glm::vec4 ShapeRendererGroup::getFillColor() const
{
    std::lock_guard<std::mutex> lock(_fillColorLock);
    return _fillColor;
}

void ShapeRendererGroup::setBorderWidth(float border)
{
    _borderWidth = border;
    for(auto child : _childPrimitives)
        child->setBorderWidth(border);
}

float ShapeRendererGroup::getBorderWidth() const
{
    return _borderWidth;
}

void ShapeRendererGroup::setFixedScreenSize(bool fixed)
{
    _fixedScreenSize = fixed;
    for(auto child : _childPrimitives)
        child->setFixedScreenSize(fixed);
}

bool ShapeRendererGroup::getFixedScreenSize() const
{
    return _fixedScreenSize;
}

void ShapeRendererGroup::setScreenOriented(bool orient)
{
    _screenOriented = orient;
    for(auto child : _childPrimitives)
        child->setScreenOriented(orient);
}

bool ShapeRendererGroup::getScreenOriented() const
{
    return _screenOriented;
}

template<>
const std::string ShaderFiles<ShapeRendererGroup>::
vertexShader = "../plugins/render/defaultShaders/primitive.vert";
template<>
const std::string ShaderFiles<ShapeRendererGroup>::
fragmentShader = "../plugins/render/defaultShaders/primitive.frag";

ShaderProgram* ShapeRendererGroup::getProgram()
{
    return getResourceManager()->shaderManager()->getProgram<ShapeRendererGroup>();
}

void ShapeRenderer::draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    UniformStateManager uniformStates(program);
    uniformStates.addState("staticTransformation", getStaticWorldTransformation());

    uniformStates.addState("fixed_screensize", getFixedScreenSize());
    uniformStates.addState("screen_oriented", getScreenOriented());
    uniformStates.addState("fillColor", getFillColor());
    uniformStates.addState("borderColor", getBorderColor());
    if(getFillColor().a > 0) {
        program->setUniform("isBorder", 0);
        drawFill(camera, config, program);
    }

    if(getBorderColor().a > 0 && getBorderWidth() > 0) {
        program->setUniform("isBorder", 1);
        glLineWidth(getBorderWidth());
        drawBorder(camera, config, program);
        glLineWidth(1);
    }
}

void ShapeRenderer::drawFill(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    glDrawArrays(GL_QUADS, 0, 4);
}

void ShapeRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisable(GL_LINE_SMOOTH);
}

LineRenderer::LineRenderer(std::initializer_list<glm::vec3> points)
    : _points(points)
{
    setFillColor(glm::vec4(0));
}


LineRenderer::~LineRenderer()
{
}

void LineRenderer::setPoints(std::vector<glm::vec3> points)
{
    _points = points;
}

void LineRenderer::init(ShaderProgram* prog)
{
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList points;
    for(auto p : _points) {
        points.push_back(p);
    }
    _vbo->data(points);
    _vbo->setPointer();
}

void LineRenderer::drawBorder(const CameraPtr&, const RenderConfig &config, ShaderProgram* program)
{
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINES, 0, _points.size());
    glDisable(GL_LINE_SMOOTH);
}

void LineRenderer::drawFill(const CameraPtr &camera,
                            const RenderConfig &config,
                            ShaderProgram* program)
{
}

QuadRenderer::QuadRenderer(float width, float height, ShapeRendererGroup *parent) :
    ShapeRenderer(parent),
    _width(width),
    _height(height)
{
}

void QuadRenderer::init(ShaderProgram* prog)
{
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList verts;
    verts.insert(begin(verts), {
                    glm::vec3(0, 0, 0),
                    glm::vec3(_width, 0, 0),
                    glm::vec3(_width, _height, 0),
                    glm::vec3(0, _height, 0)
                 });

    _vbo->data(verts);
    _vbo->setPointer();
}

GridRenderer::GridRenderer(int width, int height, int xres, int yres, ShapeRendererGroup *parent) : 
    ShapeRenderer(parent),
    _width(width),
    _height(height),
    _xres(xres),
    _yres(yres),
    _alternatingColor(glm::vec4(1))
{
    setFillColor(glm::vec4(0));

}

GridRenderer::~GridRenderer()
{
}

void GridRenderer::init(ShaderProgram* prog)
{
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList verts;

    for(int x = 0; x <= _xres; x++) {
        float nx = ((float)x) / _xres;
        nx *= 2;
        nx -= 1;

        verts.emplace_back(nx * _width / 2, - (_height / 2), 0);
        verts.emplace_back(nx * _width / 2, (_height / 2), 0);
    }

    for(int y = 0; y <= _yres; y++) {
        float ny = ((float)y) / _yres;
        ny *= 2;
        ny -= 1;

        verts.emplace_back(- _width / 2, ny * _height / 2, 0);
        verts.emplace_back(_width / 2, ny * _height / 2, 0);
    }

    _vbo->data(verts);
    _vbo->setPointer();
}

void GridRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &/*config*/, ShaderProgram* program)
{
    UniformState(program, "alternatingColor", _alternatingColor);
    UniformState(program, "gridRes", glm::ivec2(_xres, _yres));
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINES, 0, _xres * 2 + _yres * 2 + 4);
    glDisable(GL_LINE_SMOOTH);
    MTGLERROR;
}

template<>
const std::string ShaderFiles<GridRenderer>::
vertexShader = "../plugins/render/defaultShaders/grid.vert";
template<>
const std::string ShaderFiles<GridRenderer>::
fragmentShader = "../plugins/render/defaultShaders/grid.frag";

ShaderProgram* GridRenderer::getProgram()
{
    return getResourceManager()->shaderManager()->getProgram<GridRenderer>();
}

void GridRenderer::setAlternatingColor(glm::vec4 col)
{
    _alternatingColor = col;
}

DiscRenderer::DiscRenderer(ShapeRendererGroup *parent)
    : ShapeRenderer(parent), _segments(8)
{
}

void DiscRenderer::init(ShaderProgram* prog)
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList verts;

    //disc
    verts.push_back(glm::vec3());
    for(int i=0; i <= _segments; i++) {
        float pni = 2 * PI * float(i) / _segments;
        verts.push_back(glm::vec3(sin(pni), 0, cos(pni)));
    }

    _vbo->data(verts);
    _vbo->setPointer();
}

void DiscRenderer::drawFill(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, _segments + 2);
}

void DiscRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
}

CircleRenderer::CircleRenderer(ShapeRendererGroup *parent)
    : ShapeRenderer(parent), _segments(8)
{
}

void CircleRenderer::init(ShaderProgram* prog)
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList verts;

    for(int i=0; i < _segments; i++) {
        float pni = 2 * PI * float(i) / _segments;
        verts.push_back(glm::vec3(sin(pni), 0, cos(pni)));
    }

    _vbo->data(verts);
    _vbo->setPointer();
}

void CircleRenderer::drawFill(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
}

void CircleRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    glDrawArrays(GL_LINE_LOOP, 0, _segments);
}

ConeRenderer::ConeRenderer(ShapeRendererGroup *parent)
    : ShapeRenderer(parent), _segments(8)
{
    new DiscRenderer(this);
}

void ConeRenderer::init(ShaderProgram* prog)
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList verts;

    //cone
    verts.push_back(glm::vec3(0, 1, 0));
    for(int i=0; i <= _segments; i++) {
        float pni = 2 * PI * float(i) / _segments;
        verts.push_back(glm::vec3(sin(pni), 0, cos(pni)));
    }

    _vbo->data(verts);
    _vbo->setPointer();
}

void ConeRenderer::drawFill(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, _segments + 2);
}

void ConeRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
}

SphereRenderer::SphereRenderer(int u, int v) :
    _u_segments(u),
    _v_segments(v)
{
    setFillColor(glm::vec4(1));
}

void SphereRenderer::init(ShaderProgram* prog)
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _ibo = make_resource<IBO>(getResourceManager());
    _vbo->bind();
    _ibo->bind();
    prog->bindAttributeLocation(_vbo.get());

    auto polygons = std::make_shared<PolygonList>();

    VertexList verts;

    verts.emplace_back(0, 1, 0);
    verts.emplace_back(0, -1, 0);
    for(int i=1; i < _u_segments; i++) {
        float u = float(i) / _u_segments;
        float circle_radius = sin(PI * u);
        float height = cos(PI * u);
        for(int j=0; j <= _v_segments; j++) {
            float pni = 2 * PI * float(j) / _v_segments;
            verts.emplace_back(sin(pni) * circle_radius,
                               height,
                               cos(pni) * circle_radius);
        }
    }

    for(uint i = 2; i < _v_segments + 1; i += 3) {
        polygons->emplace_back(Polygon{(i + 1) % _v_segments, i, 0});
        uint offset = (_u_segments - 1) * _v_segments + 2;
        //polygons->emplace_back(Polygon{1, i + offset, i+1 + offset});
    }

    _ibo->data(polygons);
    _vbo->data(verts);
    _vbo->setPointer();
}

void SphereRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    //nothing
}

void SphereRenderer::drawFill(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    glDrawArrays(GL_POINTS, 0, (_u_segments - 2) * (_v_segments + 1) + 2);
    //glDrawArrays(GL_TRIANGLE_FAN, 0, (_u_segments - 2) * (_v_segments + 1) + 2);
    glDrawArrays(GL_TRIANGLES, 0, _v_segments);
}

SinglePointRenderer::SinglePointRenderer(ShapeRendererGroup *parent) :
    ShapeRenderer(parent),
    _pointSize(10)
{
    setFillColor(glm::vec4(0.5, 0.5, 1, 1));
    setBorderColor(glm::vec4(0));
}

void SinglePointRenderer::setPosition(glm::vec3 position)
{
    setTransformation(glm::translate(glm::mat4(), position));
}

void SinglePointRenderer::setPointSize(int size)
{
    _pointSize = size;
}

int SinglePointRenderer::getPointSize() const
{
    return _pointSize;
}

void SinglePointRenderer::init(ShaderProgram* prog)
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->geometryCache()->getIndexForAttribute("P"));
    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

    VertexList verts;
    verts.emplace_back(0, 0, 0);
    _vbo->data(verts);
    _vbo->setPointer();
}

void SinglePointRenderer::drawFill(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    UniformState state(program, "point_size", _pointSize);
    glDrawArrays(GL_POINTS, 0, 1);
}

void SinglePointRenderer::drawBorder(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program)
{
    //nothing
}

ArrowRenderer::ArrowRenderer(ShapeRendererGroup *parent)
    : ShapeRendererGroup(parent)
{
    auto line = new GL::LineRenderer{glm::vec3(0, 0, 0), 
        glm::vec3(0, 2, 0)};
    setChildPrimitive(line);
    auto *cone = new GL::ConeRenderer(this);

    cone->staticTransform(glm::translate(glm::mat4(), glm::vec3(0, 2, 0))
                                  * glm::scale(glm::mat4(), glm::vec3(.2)));
    setBorderWidth(3);
};

void ArrowRenderer::setFillColor(glm::vec4 color)
{
}

void ArrowRenderer::setBorderColor(glm::vec4 color)
{
    ShapeRendererGroup::setFillColor(color);
    ShapeRendererGroup::setBorderColor(color);
}
