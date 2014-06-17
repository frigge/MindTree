#include "primitive_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

PrimitiveRenderer::PrimitiveRenderer()
{
}

PrimitiveRenderer::~PrimitiveRenderer()
{
    auto manager = Context::getSharedContext()->getManager();
    manager->scheduleCleanUp(_vbo);
}

void PrimitiveRenderer::init()
{
    _vbo = std::make_shared<VBO>("P", 0);
}


void PrimitiveRenderer::initVAO()
{
    _vao = std::make_shared<VAO>();
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
}

std::shared_ptr<ShaderProgram> FullscreenQuadRenderer::getProgram()
{
    return _defaultProgram;
}

void FullscreenQuadRenderer::init()
{
    PrimitiveRenderer::init();
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
//    auto res = _pass->getResolution();
//    program->setUniform("xres", res.x);
//    program->setUniform("yres", res.y);
//
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    getGLError(__PRETTY_FUNCTION__);
}

std::shared_ptr<ShaderProgram> GridRenderer::_defaultProgram;

GridRenderer::GridRenderer(int width, int height, int xres, int yres)
    : _width(width), 
    _height(height), 
    _xres(xres), 
    _yres(yres),
    _thickness(1.0),
    _color(glm::vec4(1.))
{
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

std::shared_ptr<ShaderProgram> GridRenderer::getProgram()
{
    return _defaultProgram;
}

void GridRenderer::init()
{
    PrimitiveRenderer::init();
    _vbo->bind();
    _defaultProgram->bindAttributeLocation(0, "P");

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

void GridRenderer::draw(const CameraPtr camera, const RenderConfig &/*config*/, std::shared_ptr<ShaderProgram> program)
{
    glLineWidth(_thickness);
    program->setUniform("color", _color);

    glEnable(GL_LINE_SMOOTH);
    glDrawArrays(GL_LINES, 0, _xres * 2 + _yres * 2 + 4);
    glDisable(GL_LINE_SMOOTH);
    getGLError(__PRETTY_FUNCTION__);
}

void GridRenderer::setColor(glm::vec4 color)
{
    _color = color;
}

void GridRenderer::setThickness(float thickness)
{
    _thickness = thickness;
}
