#ifndef PIXEL_PLANE_MT
#define PIXEL_PLANE_MT

#include "glwrapper.h"
#include "render.h"

namespace MindTree {
namespace GL {

class PixelPlane : public Renderer
{
public:
    struct ShaderProvider {
        virtual std::shared_ptr<ShaderProgram> provideProgram() = 0;
    };

    std::shared_ptr<ShaderProgram> getProgram();

    template<typename Provider>
    void setProvider()
    {
        _provider = std::make_unique<Provider>();
    }

protected:
    virtual void init(std::shared_ptr<ShaderProgram> prog);
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    std::weak_ptr<ShaderProgram> _program;
    std::unique_ptr<ShaderProvider> _provider;
    std::shared_ptr<VBO> _vbo;
    std::shared_ptr<VBO> _coord_vbo;
};

}
}
#endif
