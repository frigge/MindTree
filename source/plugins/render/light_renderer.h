#ifndef LIGHTRENDERER_H_HP2LD7GB
#define LIGHTRENDERER_H_HP2LD7GB

#include "memory"
#include "primitive_renderer.h"

class PointLight;
class SpotLight;
class DistantLight;

namespace MindTree {

namespace GL {

class PointLightRenderer : public ShapeRendererGroup
{
public:
    PointLightRenderer(const std::shared_ptr<PointLight> l);

    virtual ~PointLightRenderer() {}

private:
    const std::shared_ptr<PointLight> _light;

};

class SpotLightRenderer : public ShapeRendererGroup
{
public:
    SpotLightRenderer(const std::shared_ptr<SpotLight> l);

private:
    const std::shared_ptr<SpotLight> _light;
};

class DistantLightRenderer : public ShapeRendererGroup
{
public:
    DistantLightRenderer(const std::shared_ptr<DistantLight> l);

private:
    const std::shared_ptr<DistantLight> _light;

};


}

}
#endif /* end of include guard: LIGHTRENDERER_H_HP2LD7GB */
