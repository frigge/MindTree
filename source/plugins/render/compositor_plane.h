#ifndef MT_GL_COMPOSITOR_PLANE_H
#define MT_GL_COMPOSITOR_PLANE_H

#include "vector"
#include "pixel_plane.h"
#include "render_block.h"

namespace MindTree {
namespace GL {
class CompositorPlane : public PixelPlane
{
public:
    CompositorPlane();

    enum class CompositType {
        ADD = 0,
        MULTIPLY,
        ALPHAOVER,
        MIX
    };

    struct CompositInfo {
        CompositType type;
        float mixValue;
        std::weak_ptr<Texture2D> texture;
        bool enabled=true;
    };

    CompositInfo& addLayer(std::weak_ptr<Texture2D> tx, float mix, CompositType type);
    CompositInfo& getInfo(std::string txName);

    const std::vector<CompositorPlane::CompositInfo>& getLayers() const;

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    std::vector<CompositInfo> _layers;
};

class Compositor : public RenderBlock
{
public:
    Compositor();

    void setProperty(std::string name, Property prop);
    void addLayer(std::weak_ptr<Texture2D> tx, float mix, CompositorPlane::CompositType type);
    void init();

 private:
    CompositorPlane *_plane;
    RenderPass *_pixelPass;
};

}
}

#endif
