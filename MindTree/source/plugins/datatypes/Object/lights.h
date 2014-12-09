#ifndef LIGHTS_H_OED9TXI3
#define LIGHTS_H_OED9TXI3

#include "object.h"

class Light;
typedef std::shared_ptr<Light> LightPtr;
class Light : public AbstractTransformable
{
public:
    enum Light_t {
        POINT,
        SPOT,
        DISTANT
    };

    Light(Light_t type, float intensity);

    AbstractTransformablePtr clone() const override;

    Light_t getLightType() const;

    inline void setIntensity(float intensity) { _intensity = intensity; }
    inline float getIntensity() const { return _intensity; }

private:
    double _intensity;
    Light_t _type;
};

class PointLight;
typedef std::shared_ptr<PointLight> PointLightPtr;
class PointLight : public Light
{
public:
    PointLight(float intensity);
    AbstractTransformablePtr clone() const override;
};

class SpotLight;
typedef std::shared_ptr<SpotLight> SpotLightPtr;
class SpotLight : public Light
{
public:
    SpotLight(float intensity, float coneangle);
    AbstractTransformablePtr clone() const override;

private:
    float _coneAngle;
};

class DistantLight;
typedef std::shared_ptr<DistantLight> DistantLightPtr;
class DistantLight : public Light
{
public:
    DistantLight(float intensity);
    AbstractTransformablePtr clone() const override;
};

#endif /* end of include guard: LIGHTS_H_OED9TXI3 */
