#include "plane.h"

using namespace glm;
using namespace MindTree;

ObjectDataPtr prim3d::createPlaneMesh(float scale)
{
    auto obj = std::make_shared<GeoObject>();
    auto mesh = std::make_shared<MeshData>();
    auto vertices = std::make_shared<VertexList>();
    auto normals = std::make_shared<VertexList>();
    auto polygons = std::make_shared<PolygonList>();

    mesh->setProperty("P", vertices);
    mesh->setProperty("N", normals);
    mesh->setProperty("polygon", polygons);
    obj->setData(mesh);

    vertices->insert(vertices->begin(), {
                     vec3(-scale, 0, -scale),
                     vec3(scale, 0, -scale),
                     vec3(scale, 0, scale),
                     vec3(-scale, 0, scale),
                     });

    normals->insert(normals->begin(), {
                     vec3(0, 1, 0),
                     vec3(0, 1, 0),
                     vec3(0, 1, 0),
                     vec3(0, 1, 0),
                     });

    polygons->push_back(Polygon{3, 2, 1, 0});
    return mesh;
}
