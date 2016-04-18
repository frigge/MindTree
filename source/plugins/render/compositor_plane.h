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
        Texture2D *texture;
        bool enabled=true;
    };

    CompositInfo& addLayer(Texture2D *tx, float mix, CompositType type);
    CompositInfo& getInfo(std::string txName);

    const std::vector<CompositorPlane::CompositInfo>& getLayers() const;
    std::vector<CompositorPlane::CompositInfo>& getLayers();

protected:
    virtual void draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program);

private:
    std::vector<CompositInfo> _layers;
};

class Compositor : public RenderBlock
{
public:
    Compositor();

    void setProperty(std::string name, Property prop);
    void addLayer(Texture2D *tx, float mix, CompositorPlane::CompositType type);
    void init();
    std::vector<std::string> getLayerNames() const;

 private:
    CompositorPlane *_plane;
    RenderPass *_pixelPass;
    RenderPass *custom_pass_;
};

}
}

#endif
