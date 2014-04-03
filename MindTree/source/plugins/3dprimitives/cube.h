#ifndef PRIM_CUBE
#define PRIM_CUBE

#include "memory"
#include "../datatypes/Object/object.h"

namespace prim3d
{
    std::shared_ptr<GeoObject> createCube(float scale);
};
#endif
