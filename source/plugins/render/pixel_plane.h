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
            auto geo = ShaderFiles<T>::geometryShader;
            auto frag = ShaderFiles<T>::fragmentShader;
            auto tessControl = ShaderFiles<T>::tessControlShader;
            auto tessEval = ShaderFiles<T>::tessEvalShader;

            if(vert)prog ->addShaderFromFile(vert, ShaderProgram::VERTEX);
            if(geo)prog ->addShaderFromFile(geo, ShaderProgram::GEOMETRY);
            if(frag)prog ->addShaderFromFile(frag, ShaderProgram::FRAGMENT);
            if(tessControl)prog ->addShaderFromFile(tessControl, ShaderProgram::TESSELATION_CONTROL);
            if(tessEval)prog ->addShaderFromFile(tessEval, ShaderProgram::TESSELATION_EVALUATION);
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
    virtual void init(std::shared_ptr<ShaderProgram> prog);
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    std::weak_ptr<ShaderProgram> _program;
    std::unique_ptr<AbstractShaderProvider> _provider;
    std::shared_ptr<VBO> _vbo;
    std::shared_ptr<VBO> _coord_vbo;
};

template<typename T> const std::string PixelPlane::ShaderFiles<T>::vertexShader{};
template<typename T> const std::string PixelPlane::ShaderFiles<T>::geometryShader{};
template<typename T> const std::string PixelPlane::ShaderFiles<T>::fragmentShader{};
template<typename T> const std::string PixelPlane::ShaderFiles<T>::tessControlShader{};
template<typename T> const std::string PixelPlane::ShaderFiles<T>::tessEvalShader{};

}
}
#endif
