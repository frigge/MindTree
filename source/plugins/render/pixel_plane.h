#ifndef PIXEL_PLANE_MT
#define PIXEL_PLANE_MT

#include "glwrapper.h"
#include "render.h"

namespace MindTree {
namespace GL {


class PixelPlane : public Renderer
{
public:
    PixelPlane(std::string shaderSrc="") : fragmentShader_{shaderSrc} {}

    ShaderProgram* getProgram();

    void setFragmentShader(std::string fragShader) {
        fragmentShader_ = fragShader;
    }

protected:
    virtual void init(ShaderProgram* prog);
    virtual void draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram *program);

private:
    ResourceHandle<VBO> _vbo;
    ResourceHandle<VBO> _coord_vbo;
    ResourceHandle<ShaderProgram> _program;
    std::string fragmentShader_;
};
}
}
#endif
