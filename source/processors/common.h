#ifndef MT_PROCESSOR_COMMON_H
#define MT_PROCESSOR_COMMON_H

#include "../plugins/datatypes/Object/dcel.h"

namespace MindTree {

std::shared_ptr<MeshData> createCylinder(unsigned sides, bool cap)
{
    auto mesh = std::make_shared<MeshData>();
    auto verts = std::make_shared<VertexList>();
    auto polys = std::make_shared<PolygonList>();

    mesh->setProperty("P", verts);
    mesh->setProperty("polygon", polys);

    double pi = acos(-1);

    //upper side
    for(int i = 0; i < sides; ++i) {
        verts->emplace_back(sin(2 * i * pi/sides) * 0.5, 1, cos(2 * i * pi/sides) * 0.5);
    }

    for(int i = 0; i < sides; ++i) {
        verts->emplace_back(sin(2 * i * pi/sides) * 0.5, 0, cos(2 * i * pi/sides) * 0.5);
    }

    if(cap) verts->emplace_back(0, 1, 0);
    if(cap) verts->emplace_back(0, 0, 0);

    for (unsigned int i = 0; i < sides; ++i) {
        Polygon p{(i+1) % sides, i, sides + i, sides + ((i+1) % sides)};
        polys->push_back(p);

        if(cap) {
            Polygon p1{i, (i+1) % sides, 2*sides};
            polys->push_back(p1);

            Polygon p2{sides + ((i+1) % sides), sides + i, 2*sides + 1};
            polys->push_back(p2);
        }
    }

    return mesh;
}

 std::vector<MeshData::Edge> createRing(unsigned sides, VertexList *verts)
{
    double pi = acos(-1);

    std::vector<MeshData::Edge> edges;

    //upper side
    auto offset = verts->size();
    for(int i = 0; i < sides; ++i) {
        verts->emplace_back(sin(2 * i * pi/sides) * 0.5, 0, cos(2 * i * pi/sides) * 0.5);
        if(i > 0) {
            edges.emplace_back(offset + i-1, offset + i);
        }
    }
    edges.emplace_back(offset + sides - 1, offset);

    return edges;
}

dcel::Edge* createRingDCEL(unsigned sides, dcel::Adapter *dcel_data)
{
    double pi = std::acos(-1);

    dcel::Vertex *lastVert = nullptr, *firstVert = nullptr;
    //create simple disk shape
    for(int i = 0; i < sides; ++i) {
        auto *vert = dcel_data->newVertex();

        if(i == 0) {
            lastVert = vert;
            firstVert = vert;
        }
        glm::vec3 p(sin(2 * pi * i/sides) * 0.5,
                    0,
                    cos(2 * pi * i/sides) * 0.5);
        vert->set("P", p);
        if(i == 0) continue;

        dcel_data->connect(vert, lastVert);
        lastVert = vert;
    }

    auto *edge = dcel_data->connect(lastVert, firstVert);
    dcel_data->updateMesh();

    return edge;
}

}
#endif
