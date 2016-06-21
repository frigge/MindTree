#ifndef MT_PROCESSOR_COMMON_H
#define MT_PROCESSOR_COMMON_H

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

}
#endif
