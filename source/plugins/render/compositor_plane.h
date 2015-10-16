#ifndef MT_GL_COMPOSITOR_PLANE_H
#define MT_GL_COMPOSITOR_PLANE_H

#include "vector"
#include "pixel_plane.h"

namespace MindTree {
namespace GL {
class CompositorPlane : public PixelPlane
{
public:
    struct CompositInfo {
        enum class CompositType {
            ADD,
            MULTIPLY,
            ALPHAOVER,
            MIX
        };

        CompositType type;
        float mixValue;
        std::weak_ptr<Texture2D> texture;
    };

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    std::vector<CompositInfo> _layers;
};
}
}

#endif
