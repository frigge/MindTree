#include "pixel_plane.h"

using namespace MindTree;
using namespace GL;

std::shared_ptr<ShaderProgram> PixelPlane::getProgram()
{
    std::shared_ptr<ShaderProgram> prog;
    if(_program.expired() && _provider) {
        prog = _provider->provideProgram();

        _program = prog;
    }
    return prog;
}

void PixelPlane::init(ShaderProgram* prog)
{
    _vbo = std::make_shared<VBO>("P");
    _coord_vbo = std::make_shared<VBO>("st");
    _coord_vbo->overrideIndex(1);

    _vbo->bind();
    prog->bindAttributeLocation(_vbo);
    
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
    prog->bindAttributeLocation(_coord_vbo);
    _coord_vbo->data(coords);
    _coord_vbo->setPointer();
}

void PixelPlane::draw(const CameraPtr /* camera */, 
                                  const RenderConfig& /* config */, 
                                  std::shared_ptr<ShaderProgram> program)
{
    UniformState(program, "bgcolor", glm::vec4(.275, .275, .275, 1.));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    MTGLERROR;
}
