#define GLM_SWIZZLE
#include "GL/glew.h"
#include "renderpass.h"
#include "primitive_renderer.h"

#define PI 3.14159265359

using namespace MindTree;
using namespace MindTree::GL;

std::shared_ptr<ShaderProgram> PrimitiveRenderer::_defaultProgram;

PrimitiveRenderer::PrimitiveRenderer()
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
}

PrimitiveRenderer::~PrimitiveRenderer()
{
    auto manager = Context::getSharedContext()->getManager();
    manager->scheduleCleanUp(_vbo);
}

void PrimitiveRenderer::setParentPrimitive(PrimitiveRenderer *renderer)
{
    setParent(renderer);
    renderer->setChildPrimitive(this);
}

void PrimitiveRenderer::setChildPrimitive(PrimitiveRenderer *renderer)
{
    addChild(renderer);
    _childPrimitives.push_back(renderer);
}

void PrimitiveRenderer::setStaticTransformation(glm::mat4 trans)
{
    _staticTransformation = trans;
}

glm::mat4 PrimitiveRenderer::getStaticWorldTransformation()
{
    auto parent = dynamic_cast<PrimitiveRenderer*>(getParent());
    if (parent) {
        return parent->getStaticWorldTransformation() * _staticTransformation;
    }
    return _staticTransformation;
}

void PrimitiveRenderer::init()
{
    _vbo = std::make_shared<VBO>("P", 0);
    _defaultProgram->bindAttributeLocation(0, "P");
}

void PrimitiveRenderer::initVAO()
{
    _vao = std::make_shared<VAO>();
}

void PrimitiveRenderer::setFillColor(glm::vec4 color)
{
    _fillColor = color;
    for(auto child : _childPrimitives)
        child->setFillColor(color);
}

void PrimitiveRenderer::setBorderColor(glm::vec4 color)
{
    _borderColor = color;
    for(auto child : _childPrimitives)
        child->setBorderColor(color);
}

void PrimitiveRenderer::setBorderWidth(float border)
{
    _borderWidth = border;
    for(auto child : _childPrimitives)
        child->setBorderWidth(border);
}

float PrimitiveRenderer::getBorderWidth()
{
    return _borderWidth;
}

void PrimitiveRenderer::setFixedScreenSize(bool fixed)
{
    _fixedScreenSize = fixed;
    for(auto child : _childPrimitives)
        child->setFixedScreenSize(fixed);
}

bool PrimitiveRenderer::getFixedScreenSize()
{
    return _fixedScreenSize;
}

void PrimitiveRenderer::setScreenOriented(bool orient)
{
    _screenOriented = orient;
    for(auto child : _childPrimitives)
        child->setScreenOriented(orient);
}

bool PrimitiveRenderer::getScreenOriented()
{
    return _screenOriented;
}

std::shared_ptr<ShaderProgram> PrimitiveRenderer::getProgram()
{
    return _defaultProgram;
}

void PrimitiveRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    program->setUniform("fixed_screensize", _fixedScreenSize);
    program->setUniform("screen_oriented", _screenOriented);
    if(_fillColor.a > 0) {
        program->setUniform("isBorder", 0);
        program->setUniform("fillColor", _fillColor);
        drawFill(camera, config, program);
    }

    if(_borderColor.a > 0 && _borderWidth > 0) {
        program->setUniform("isBorder", 1);
        program->setUniform("borderColor", _borderColor);
        glLineWidth(_borderWidth);
        drawBorder(camera, config, program);
        glLineWidth(1);
    }
}

void PrimitiveRenderer::drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glDrawArrays(GL_QUADS, 0, 4);
}

void PrimitiveRenderer::drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisable(GL_LINE_SMOOTH);
}

LineRenderer::LineRenderer()
{
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
    PrimitiveRenderer::init();
    _vbo->bind();

    auto staticWorldTrans = getStaticWorldTransformation();
    VertexList points;
    for(auto p : _points) {
        points.push_back((staticWorldTrans * glm::vec4(p, 1)).xyz());
    }
    _vbo->data(points);
}

void LineRenderer::drawBorder(const CameraPtr, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINE_STRIP, 0, _points.size());
    glDisable(GL_LINE_SMOOTH);
}

Widget3dRenderer::Widget3dRenderer()
    : PrimitiveRenderer()
{
}

Widget3dRenderer::~Widget3dRenderer()
{
}

void Widget3dRenderer::initVAO()
{
    /* nothing to do here */
}

void Widget3dRenderer::init()
{
    /* nothing to do here */
}

void Widget3dRenderer::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
}

QuadRenderer::QuadRenderer(float width, float height)
    : _width(width), 
    _height(height)
{
}

void QuadRenderer::init()
{
    PrimitiveRenderer::init();

    _vbo->bind();

    auto trans = getStaticWorldTransformation();
    VertexList verts;
    verts.insert(begin(verts), {
                    (trans * glm::vec4(0, 0, 0, 1)).xyz(),
                    (trans * glm::vec4(_width, 0, 0, 1)).xyz(),
                    (trans * glm::vec4(_width, _height, 0, 1)).xyz(),
                    (trans * glm::vec4(0, _height, 0, 1)).xyz()
                 });

    _vbo->data(verts);
}

std::shared_ptr<ShaderProgram> FullscreenQuadRenderer::_defaultProgram;

FullscreenQuadRenderer::FullscreenQuadRenderer(RenderPass *pass)
    : _pass(pass)
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
    auto manager = Context::getSharedContext()->getManager();
    manager->scheduleCleanUp(_vbo);
    manager->scheduleCleanUp(_coord_vbo);
}

std::shared_ptr<ShaderProgram> FullscreenQuadRenderer::getProgram()
{
    return _defaultProgram;
}

void FullscreenQuadRenderer::initVAO()
{
    _vao = std::make_shared<VAO>();
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

    _coord_vbo->bind();
    _defaultProgram->bindAttributeLocation(1, "_st");
    _coord_vbo->data(coords);
}

void FullscreenQuadRenderer::draw(const CameraPtr /* camera */, 
                                  const RenderConfig& /* config */, 
                                  std::shared_ptr<ShaderProgram> program)
{
    program->setUniform("bgcolor", glm::vec4(.275, .275, .275, 1.));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    getGLError(__PRETTY_FUNCTION__);
}

std::shared_ptr<ShaderProgram> GridRenderer::_defaultProgram;

GridRenderer::GridRenderer(int width, int height, int xres, int yres)
    : _width(width), 
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
    PrimitiveRenderer::init();
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
}

void GridRenderer::drawBorder(const CameraPtr camera, const RenderConfig &/*config*/, std::shared_ptr<ShaderProgram> program)
{
    program->setUniform("alternatingColor", _alternatingColor);
    program->setUniform("gridRes", glm::ivec2(_xres, _yres));
    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINES, 0, _xres * 2 + _yres * 2 + 4);
    glDisable(GL_LINE_SMOOTH);
    getGLError(__PRETTY_FUNCTION__);
}

std::shared_ptr<ShaderProgram> GridRenderer::getProgram()
{
    return _defaultProgram;
}

void GridRenderer::setAlternatingColor(glm::vec4 col)
{
    _alternatingColor = col;
}

ConeRenderer::ConeRenderer(float height,  float radius, int segments)
    : _height(height), _radius(radius), _segments(segments)
{

}

ConeRenderer::~ConeRenderer()
{
}

void ConeRenderer::init()
{
    PrimitiveRenderer::init();
    _vbo->bind();

    VertexList verts;

    auto staticWorldTrans = getStaticWorldTransformation();
    verts.push_back((staticWorldTrans * glm::vec4(0, _height, 0, 1)).xyz());
    for(int i=0; i <= _segments; i++) {
        float pni = 2 * PI * float(i) / _segments;
        verts.push_back((staticWorldTrans * glm::vec4(sin(pni) * _radius, 0, cos(pni) * _radius, 1)).xyz());
    }

    verts.push_back((staticWorldTrans * glm::vec4(0, 0, 0, 1)).xyz());
    for(int i=0; i <= _segments; i++) {
        float pni = 2 * PI * float(i) / _segments;
        verts.push_back((staticWorldTrans * glm::vec4(sin(pni) * _radius, 0, cos(pni) * _radius, 1)).xyz());
    }

    _vbo->data(verts);
}

void ConeRenderer::drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{

}

void ConeRenderer::drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, _segments * 2 + 4);
}
