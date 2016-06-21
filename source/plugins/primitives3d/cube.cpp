#include "cube.h"

using namespace glm;

std::shared_ptr<ObjectData> prim3d::createCubeMesh(float scale)
{
    auto mesh = std::make_shared<MeshData>();

    auto vertices = std::make_shared<VertexList>();

    vertices->insert(vertices->begin(), {
                     vec3(-scale, -scale, -scale),
                     vec3(scale, -scale, -scale),
                     vec3(scale, scale, -scale),
                     vec3(-scale, scale, -scale),

                     vec3(-scale, -scale, scale),
                     vec3(scale, -scale, scale),
                     vec3(scale, scale, scale),
                     vec3(-scale, scale, scale)
    });

    auto polygons = std::make_shared<PolygonList>();

    polygons->insert(polygons->begin(), {
                     Polygon{3, 2, 1, 0}, //front
                     Polygon{4, 5, 6, 7}, //back

                     Polygon{0, 1, 5, 4}, //bottom
                     Polygon{2, 3, 7, 6}, //top

                     Polygon{3, 0, 4, 7}, //right
                     Polygon{1, 2, 6, 5} //left
    });

    mesh->setProperty("P", vertices);
    mesh->setProperty("polygon", polygons);
    mesh->computeVertexNormals();

    return mesh;
}
