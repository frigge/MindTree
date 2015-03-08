#ifndef RSM_COMPUTATION_PLANE_H
#define RSM_COMPUTATION_PLANE_H

#include "light_accumulation_plane.h"
namespace MindTree {
namespace GL {

class RSMAccumulationPlane : public LightAccumulationPlane
{
public:
    std::shared_ptr<ShaderProgram> getProgram();

private:
    static std::weak_ptr<ShaderProgram> _defaultProgram;
};

}
}
#endif
