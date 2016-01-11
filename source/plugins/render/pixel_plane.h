#ifndef PIXEL_PLANE_MT
#define PIXEL_PLANE_MT

#include "glwrapper.h"
#include "render.h"

namespace MindTree {
namespace GL {

class PixelPlane : public Renderer
{
public:
    struct AbstractShaderProvider {
        virtual std::shared_ptr<ShaderProgram> provideProgram() = 0;
    };

    template<typename T>
    struct ShaderFiles {
        static const std::string vertexShader;
        static const std::string geometryShader;
        static const std::string fragmentShader;
        static const std::string tessControlShader;
        static const std::string tessEvalShader;
    };

    template<typename T>
    struct ShaderProvider : AbstractShaderProvider {
        std::shared_ptr<ShaderProgram> provideProgram() override {
            auto prog = std::make_shared<ShaderProgram>();
            auto vert = ShaderFiles<T>::vertexShader;
            auto frag = ShaderFiles<T>::fragmentShader;

            if(vert != "")prog ->addShaderFromFile(vert, ShaderProgram::VERTEX);
            if(frag != "")prog ->addShaderFromFile(frag, ShaderProgram::FRAGMENT);
            return prog;
        }
    };
    std::shared_ptr<ShaderProgram> getProgram();

    template<typename Provider>
    void setProvider()
    {
        _provider = std::make_unique<ShaderProvider<Provider>>();
    }

protected:
    virtual void init(ShaderProgram* prog);
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    std::weak_ptr<ShaderProgram> _program;
    std::unique_ptr<AbstractShaderProvider> _provider;
    std::shared_ptr<VBO> _vbo;
    std::shared_ptr<VBO> _coord_vbo;
};

template<typename T> const std::string PixelPlane::ShaderFiles<T>::
    vertexShader{ "../plugins/render/defaultShaders/fullscreenquad.vert"};
template<typename T> const std::string PixelPlane::ShaderFiles<T>::fragmentShader{};
}
}
#endif
