#define GLM_SWIZZLE
#include "glm/gtc/matrix_transform.hpp"
#include "GL/glew.h"
#include "renderpass.h"
#include "rendermanager.h"
#include "primitive_renderer.h"

#define PI 3.14159265359

using namespace MindTree;
using namespace MindTree::GL;

std::shared_ptr<ShaderProgram> ShapeRendererGroup::_defaultProgram;

ShapeRendererGroup::ShapeRendererGroup(ShapeRendererGroup *parent)
    : _fillColor(glm::vec4(1)),
    _borderColor(glm::vec4(1)),
    _borderWidth(1.),
    _fixedScreenSize(false),
    _screenOriented(false)
{
    if(!_defaultProgram) {
        _defaultProgram = std::make_shared<ShaderProgram>();

        _defaultProgram
            ->addShaderFromFile("../plugins/render/defaultShaders/primitive.vert", 
                                ShaderProgram::VERTEX);
        _defaultProgram
            ->addShaderFromFile("../plugins/render/defaultShaders/primitive.frag", 
                                ShaderProgram::FRAGMENT);
    }

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

void ShapeRendererGroup::init()
{
    /* nothing to do here */
}

void ShapeRendererGroup::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
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

std::shared_ptr<ShaderProgram> ShapeRendererGroup::getProgram()
{
    return _defaultProgram;
}

ShapeRenderer::~ShapeRenderer()
{
}

void ShapeRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    program->setUniform("staticTransformation", getStaticWorldTransformation());

    program->setUniform("fixed_screensize", getFixedScreenSize());
    program->setUniform("screen_oriented", getScreenOriented());
    if(getFillColor().a > 0) {
        program->setUniform("isBorder", 0);
        program->setUniform("fillColor", getFillColor());
        drawFill(camera, config, program);
    }

    if(getBorderColor().a > 0 && getBorderWidth() > 0) {
        program->setUniform("isBorder", 1);
        program->setUniform("borderColor", getBorderColor());
        glLineWidth(getBorderWidth());
        drawBorder(camera, config, program);
        glLineWidth(1);
    }
}

void ShapeRenderer::drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glDrawArrays(GL_QUADS, 0, 4);
}

void ShapeRenderer::drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
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

void LineRenderer::setPoints(std::initializer_list<glm::vec3> points)
{
    _points = points;
}

void LineRenderer::init()
{
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
    _vbo->bind();

    VertexList points;
    for(auto p : _points) {
        points.push_back(p);
    }
    _vbo->data(points);
    _vbo->setPointer();
}

void LineRenderer::drawBorder(const CameraPtr, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINE_STRIP, 0, _points.size());
    glDisable(GL_LINE_SMOOTH);
}

QuadRenderer::QuadRenderer(float width, float height, ShapeRendererGroup *parent) : 
    ShapeRenderer(parent),
    _width(width),
    _height(height)
{
}

void QuadRenderer::init()
{
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
    _vbo->bind();

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

std::shared_ptr<ShaderProgram> FullscreenQuadRenderer::_defaultProgram;

FullscreenQuadRenderer::FullscreenQuadRenderer()
{
    if(!_defaultProgram) {
        _defaultProgram = std::make_shared<ShaderProgram>();

        _defaultProgram
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        _defaultProgram
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.frag", 
                                ShaderProgram::FRAGMENT);
    }
}

FullscreenQuadRenderer::~FullscreenQuadRenderer()
{
    auto manager = RenderManager::getResourceManager();
    manager->scheduleCleanUp(_vbo);
    manager->scheduleCleanUp(_coord_vbo);
}

std::shared_ptr<ShaderProgram> FullscreenQuadRenderer::getProgram()
{
    return _defaultProgram;
}

void FullscreenQuadRenderer::init()
{
    _vbo = std::make_shared<VBO>("P", 0);
    _coord_vbo = std::make_shared<VBO>("st", 1);

    _vbo->bind();
    _defaultProgram->bindAttributeLocation(0, "P");
    
    VertexList verts;
    verts.insert(verts.begin(), {
                    glm::vec3(1, 1, 0),
                    glm::vec3(-1, 1, 0),
                    glm::vec3(1, -1, 0),
                    glm::vec3(-1, -1, 0)
                  });

    auto coords = std::vector<glm::vec2>();
    coords.insert(coords.begin(), {
                    glm::vec2(1, 1),
                    glm::vec2(0, 1),
                    glm::vec2(1, 0),
                    glm::vec2(0, 0)
                 });

    _vbo->data(verts);
    _vbo->setPointer();

    _coord_vbo->bind();
    _defaultProgram->bindAttributeLocation(1, "_st");
    _coord_vbo->data(coords);
    _coord_vbo->setPointer();
}

void FullscreenQuadRenderer::draw(const CameraPtr /* camera */, 
                                  const RenderConfig& /* config */, 
                                  std::shared_ptr<ShaderProgram> program)
{
    program->setUniform("bgcolor", glm::vec4(.275, .275, .275, 1.));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    MTGLERROR;
}

std::shared_ptr<ShaderProgram> GridRenderer::_defaultProgram;

GridRenderer::GridRenderer(int width, int height, int xres, int yres, ShapeRendererGroup *parent) : 
    ShapeRenderer(parent),
    _width(width),
    _height(height),
    _xres(xres),
    _yres(yres),
    _alternatingColor(glm::vec4(1))
{
    setFillColor(glm::vec4(0));

    if(!_defaultProgram) {
        _defaultProgram = std::make_shared<ShaderProgram>();

        _defaultProgram
            ->addShaderFromFile("../plugins/render/defaultShaders/grid.vert", 
                                ShaderProgram::VERTEX);
        _defaultProgram
            ->addShaderFromFile("../plugins/render/defaultShaders/grid.frag", 
                                ShaderProgram::FRAGMENT);
    }
}

GridRenderer::~GridRenderer()
{
}

void GridRenderer::init()
{
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
    _vbo->bind();

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

void GridRenderer::drawBorder(const CameraPtr camera, const RenderConfig &/*config*/, std::shared_ptr<ShaderProgram> program)
{
    program->setUniform("alternatingColor", _alternatingColor);
    program->setUniform("gridRes", glm::ivec2(_xres, _yres));
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINES, 0, _xres * 2 + _yres * 2 + 4);
    glDisable(GL_LINE_SMOOTH);
    MTGLERROR;
}

std::shared_ptr<ShaderProgram> GridRenderer::getProgram()
{
    return _defaultProgram;
}

void GridRenderer::setAlternatingColor(glm::vec4 col)
{
    _alternatingColor = col;
}

std::shared_ptr<VBO> DiscRenderer::_vbo;

DiscRenderer::DiscRenderer(ShapeRendererGroup *parent)
    : ShapeRenderer(parent), _segments(8)
{
}

void DiscRenderer::init()
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
    _vbo->bind();

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

void DiscRenderer::drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, _segments + 2);
}

void DiscRenderer::drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
}

std::shared_ptr<VBO> CircleRenderer::_vbo;
CircleRenderer::CircleRenderer(ShapeRendererGroup *parent)
    : ShapeRenderer(parent), _segments(8)
{
}

void CircleRenderer::init()
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
    _vbo->bind();

    VertexList verts;

    for(int i=0; i < _segments; i++) {
        float pni = 2 * PI * float(i) / _segments;
        verts.push_back(glm::vec3(sin(pni), 0, cos(pni)));
    }

    _vbo->data(verts);
    _vbo->setPointer();
}

void CircleRenderer::drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
}

void CircleRenderer::drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glDrawArrays(GL_LINE_LOOP, 0, _segments);
}

std::shared_ptr<VBO> ConeRenderer::_vbo;
ConeRenderer::ConeRenderer(ShapeRendererGroup *parent)
    : ShapeRenderer(parent), _segments(8)
{
    new CircleRenderer(this);
}

void ConeRenderer::init()
{
    if(_vbo) {
        _vbo->bind();
        _vbo->setPointer();
        return;
    }
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
    _vbo->bind();

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

void ConeRenderer::drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, _segments + 2);
}

void ConeRenderer::drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
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
