#ifndef MT_GEOOBJECT_RENDERER_H
#define MT_GEOOBJECT_RENDERER_H

#include "render.h"

namespace MindTree {
namespace GL {

class GeoObjectRenderer : public Renderer
{
public:
    GeoObjectRenderer(std::shared_ptr<GeoObject> o);
    virtual ~GeoObjectRenderer();

protected:
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

    void init(std::shared_ptr<ShaderProgram> prog);
    virtual void initCustom();

    std::shared_ptr<GeoObject> obj;

private:
    void setUniforms();
};

}
}

#endif
