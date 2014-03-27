#ifndef PRIM_CUBE
#define PRIM_CUBE

#include "memory"
#include "../datatypes/Object/object.h"

namespace prim3d
{
    std::shared_ptr<Object> createCube(float scale);
};
#endif
