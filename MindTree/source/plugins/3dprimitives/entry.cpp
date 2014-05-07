#include "sphere.h"
#include "cube.h"
#include "plane.h"
#include "cylinder.h"
#include "torus.h"

#include "boost/python.hpp"

#include "data/cache_main.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(prim3d)
{
    auto cubeproc = [] (DataCache *cache) {
        float scale = cache->getData(0).getData<double>();
        auto cube = prim3d::createCube(scale);
        cache->pushData(cube);
    };

    auto planeproc = [] (DataCache *cache) {
        float scale = cache->getData(0).getData<double>();
        auto plane = prim3d::createPlane(scale);
        cache->pushData(plane);
    };

    DataCache::addProcessor("SCENEOBJECT", "CUBE", new CacheProcessor(cubeproc));
    DataCache::addProcessor("SCENEOBJECT", "PLANE", new CacheProcessor(planeproc));
}
