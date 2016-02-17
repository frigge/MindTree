#include "pixel_plane.h"

using namespace MindTree;
using namespace GL;

template<>
const std::string ShaderFiles<PixelPlane>::
vertexShader{ "../plugins/render/defaultShaders/fullscreenquad.vert"};

ShaderProgram* PixelPlane::getProgram() {
    _program =  ShaderProvider<PixelPlane>::provideProgram(getResourceManager());
    _program->addShaderFromFile(fragmentShader_, ShaderProgram::FRAGMENT);
    return _program.get();
}

void PixelPlane::init(ShaderProgram* prog)
{
    _vbo = make_resource<VBO>(getResourceManager(),
                              "P");
    _vbo->overrideIndex(getResourceManager()->getIndexForAttribute("P"));
    _coord_vbo = make_resource<VBO>(getResourceManager(),
                                    "st");
    _coord_vbo->overrideIndex(1);

    _vbo->bind();
    prog->bindAttributeLocation(_vbo.get());

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
    prog->bindAttributeLocation(_coord_vbo.get());
    _coord_vbo->data(coords);
    _coord_vbo->setPointer();
}

void PixelPlane::draw(const CameraPtr& /* camera */,
                                  const RenderConfig& /* config */,
                                  ShaderProgram *program)
{
    UniformState state(program, "bgcolor", glm::vec4(.275, .275, .275, 1.));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    MTGLERROR;
}
