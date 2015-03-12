#ifndef RSM_COMPUTATION_PLANE_H
#define RSM_COMPUTATION_PLANE_H

#include "light_accumulation_plane.h"
namespace MindTree {
namespace GL {

class RSMIndirectPlane : public LightAccumulationPlane
{
public:
    RSMIndirectPlane();

    void setSearchRadius(double radius);
    void setIntensity(double intensity);

protected:
    void init(std::shared_ptr<ShaderProgram> program);
    void drawLight(const LightPtr light, 
                   std::shared_ptr<ShaderProgram> program) const;

private:
    std::shared_ptr<Texture2D> _samplingPattern;
    std::atomic<double> _searchRadius;
    std::atomic<double> _intensity;
};

}
}
#endif
