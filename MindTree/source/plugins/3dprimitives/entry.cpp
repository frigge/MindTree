#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
#include "torus.h"

#include "boost/python.hpp"

#include "data/cache_main.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(prim3d)
{
    auto cubeproc = [] (DataCache *cache) {
        auto grp = std::make_shared<Group>();

        float scale = cache->getData(0).getData<double>();
        glm::vec4 pointColor = cache->getData(1).getData<glm::vec4>();
        int pointSize = cache->getData(2).getData<int>();
        glm::vec4 edgeColor = cache->getData(3).getData<glm::vec4>();
        glm::vec4 polyColor = cache->getData(4).getData<glm::vec4>();
        bool drawVertexNormal = cache->getData(5).getData<bool>();
        bool drawPolyNormal = cache->getData(6).getData<bool>();
        int vertexNormalWidth = cache->getData(7).getData<int>();
        glm::vec4 vertexNormalColor = cache->getData(8).getData<glm::vec4>();
        double vnSize = cache->getData(9).getData<double>();

        auto cube = prim3d::createCube(scale);
        cube->setProperty("display.pointColor", pointColor);
        cube->setProperty("display.pointSize", pointSize);
        cube->setProperty("display.edgeColor", edgeColor);
        cube->setProperty("display.polyColor", polyColor);
        cube->setProperty("display.drawVertexNormal", drawVertexNormal);
        cube->setProperty("display.drawPolyNormal", drawPolyNormal);
        cube->setProperty("display.vertexNormalWidth", vertexNormalWidth);
        cube->setProperty("display.vertexNormalColor", vertexNormalColor);
        cube->setProperty("display.vertexNormalSize", vnSize);

        grp->addMember(cube);
        cache->pushData(grp);
    };

    DataCache::addProcessor("GROUPDATA", "CUBE", new CacheProcessor(cubeproc));
}
