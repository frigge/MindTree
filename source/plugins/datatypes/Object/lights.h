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

    struct ShadowInfo {
        ShadowInfo() : _enabled(false), _size(256, 256), _near(1), _far(10) {}
        ShadowInfo(bool en, glm::ivec2 s) : _enabled(en), _size(s) {}
        bool _enabled;
        glm::ivec2 _size;
        double _near;
        double _far;
    };

    Light(Light_t type, double intensity, glm::vec4 color);

    AbstractTransformablePtr clone() const override;

    Light_t getLightType() const;

    void setShadowInfo(ShadowInfo info);
    Light::ShadowInfo getShadowInfo() const;

    inline void setIntensity(double intensity) { _intensity = intensity; }
    inline double getIntensity() const { return _intensity; }
    inline glm::vec4 getColor() const { return _color; }
    inline void setColor(const glm::vec4 &color) { _color = color; }

private:
    double _intensity;
    Light_t _type;
    glm::vec4 _color;

    ShadowInfo _shadowInfo;
};

class PointLight;
typedef std::shared_ptr<PointLight> PointLightPtr;
class PointLight : public Light
{
public:
    PointLight(double intensity, glm::vec4 color);
    AbstractTransformablePtr clone() const override;
};

class SpotLight;
typedef std::shared_ptr<SpotLight> SpotLightPtr;
class SpotLight : public Light
{
public:
    SpotLight(double intensity, glm::vec4 color, double coneangle);
    AbstractTransformablePtr clone() const override;

    inline double getConeAngle() const
    {
        return _coneAngle;
    }

private:
    double _coneAngle;
};

class DistantLight;
typedef std::shared_ptr<DistantLight> DistantLightPtr;
class DistantLight : public Light
{
public:
    DistantLight(double intensity, glm::vec4 color);
    AbstractTransformablePtr clone() const override;
};

#endif /* end of include guard: LIGHTS_H_OED9TXI3 */
