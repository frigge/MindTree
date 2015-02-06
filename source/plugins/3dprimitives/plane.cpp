#include "plane.h"

using namespace glm;
using namespace MindTree;

GeoObjectPtr prim3d::createPlane(float scale)
{
    auto obj = std::make_shared<GeoObject>();
    auto mesh = std::make_shared<MeshData>();
    auto vertices = std::make_shared<VertexList>();
    auto polygons = std::make_shared<PolygonList>();

    mesh->setProperty("P", vertices);
    mesh->setProperty("polygon", polygons);
    obj->setData(mesh);

    vertices->insert(vertices->begin(), {
                     vec3(-scale, 0, -scale),
                     vec3(scale, 0, -scale),
                     vec3(scale, 0, scale),
                     vec3(-scale, 0, scale),
                     });

    polygons->push_back(Polygon{3, 2, 1, 0});

    obj->setName("Plane");
    return obj;
}
