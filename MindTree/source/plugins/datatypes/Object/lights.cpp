#include "lights.h"

Light::Light(Light_t type, float intensity)
    : AbstractTransformable(LIGHT), _type(type), _intensity(intensity)
{
}

AbstractTransformablePtr Light::clone() const
{
    auto *obj = new Light(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
}

Light::Light_t Light::getLightType() const
{
    return _type;
}

PointLight::PointLight(float intensity)
    : Light(POINT, intensity)
{
}

AbstractTransformablePtr PointLight::clone() const
{
    auto *obj = new PointLight(*this);
    return std::shared_ptr<PointLight>(obj);
}

SpotLight::SpotLight(float intensity, float coneangle)
    : Light(SPOT, intensity), _coneAngle(coneangle)
{
}

AbstractTransformablePtr SpotLight::clone() const
{
    auto *obj = new SpotLight(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
}

DistantLight::DistantLight(float intensity)
    : Light(DISTANT, intensity)
{
}

AbstractTransformablePtr DistantLight::clone() const
{
    auto *obj = new DistantLight(*this);
    return std::shared_ptr<AbstractTransformable>(obj);
}

