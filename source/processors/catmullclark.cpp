#define GLM_SWIZZLE
#include <set>
#include <unordered_set>

#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void inheritAttributes(PropertyMap *attributes, const PropertyMap &old, uint oldi, uint i)
{
    for(const auto &prop : old){
        if(!attributes->contains(prop.first)) {
            (*attributes)[prop.first] = Property::getItem(prop.second, 0).createList(i);
        }
        Property::setItem((*attributes)[prop.first],
                          i,
                          Property::getItem(prop.second, oldi));
    }
}

void subd(DataCache* cache)
{
    auto base = cache->getData(0).getData<std::shared_ptr<MeshData>>();
    auto iterations = cache->getData(1).getData<int>();

    auto mesh = std::make_shared<MeshData>();
    auto polys = base->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto verts = base->getProperty("P").getData<std::shared_ptr<VertexList>>();

    PropertyMap poly_properties;
    auto original_props = base->getProperties();

    for(const auto &prop : original_props) {
        if(prop.second.isList() && prop.second.size() == polys->size()
           && prop.first != "polygon") {
            poly_properties[prop.first] = prop.second;
        }
    }

    for(int i = 0; i < iterations; ++i) {
        auto points = std::make_shared<VertexList>(*verts);
        auto polygons = std::make_shared<PolygonList>();

        PropertyMap props = poly_properties;
        uint vertCnt = points->size();

        //face points
        std::vector<uint> face_points;
        for (const auto &p : *polys) {
            glm::vec3 fp;

            for(int i = 0; i < p.size(); ++i) {
                fp += points->at(p[i]);
            }
            fp /= p.size();
            face_points.push_back(points->size());
            points->push_back(fp);
        }

        //edgepoints
        std::unordered_map<MeshData::Edge, uint> edge_points;
        int pi = 0;
        for (const auto &p : *polys) {
            for(int i = 0; i < p.size(); ++i) {
                MeshData::Edge e(p[i], p[(i + 1) % p.size()]);

                auto it = edge_points.find(e);
                if(edge_points.find(e) != edge_points.end()) {
                    points->at(it->second) += points->at(face_points[pi]);
                    continue;
                }

                glm::vec3 v0 = points->at(e.v0());
                glm::vec3 v1 = points->at(e.v1());
                glm::vec3 fp = points->at(face_points[pi]);
                edge_points[e] = points->size();
                points->push_back(v0 + v1 + fp);
            }
            pi++;
        }

        for(auto &p : edge_points) {
            points->at(p.second) /= 4.f;
        }

        //calculate pos of old points
        struct Adjacency {
            glm::vec3 pos{0};
            uint size{0};
        };

        std::vector<Adjacency> R(vertCnt);
        for (const auto &p : edge_points) {
            //calculate midpoint
            glm::vec3 mid = points->at(p.first.v0()) + points->at(p.first.v1());
            mid *= 0.5;
            R[p.first.v0()].pos += mid;
            R[p.first.v1()].pos += mid;
            R[p.first.v0()].size++;
            R[p.first.v1()].size++;
        }

        for(int i = 0; i < R.size(); i++)
            R[i].pos /= R[i].size;

        std::vector<Adjacency> F(vertCnt);
        pi = 0;
        for(const auto &p : *polys) {
            for(uint i : p) {
                F[i].pos += points->at(face_points[pi]);
                F[i].size++;
            }
            pi++;
        }

        for(int i = 0; i < F.size(); i++)
            F[i].pos /= F[i].size;

        for(int i = 0; i < F.size(); i++) {
            auto P = points->at(i);
            auto n = F[i].size;
            points->at(i) = (F[i].pos + 2.f * R[i].pos + (n - 3.f) * P) / float(n);
        }

        for(int i = 0; i < face_points.size(); i++) {
            uint p = face_points[i];
            Polygon oldPoly = polys->at(i);
            for(int j = 0; j < oldPoly.size(); j++) {
                Polygon poly({p});
                MeshData::Edge e1(oldPoly[j], oldPoly[(j+1) % oldPoly.size()]);
                MeshData::Edge e2(oldPoly[(j+1) % oldPoly.size()], oldPoly[(j+2) % oldPoly.size()]);
                poly.push_back(edge_points[e1]);
                poly.push_back(e1.v1());
                poly.push_back(edge_points[e2]);
                inheritAttributes(&props, poly_properties, i, polygons->size());
                polygons->push_back(poly);
            }
        }
        polys = polygons;
        verts = points;
        poly_properties = std::move(props);
    }

    mesh->setProperty("P", verts);
    mesh->setProperty("polygon", polys);

    for(const auto &p : poly_properties)
        mesh->setProperty(p.first, p.second);

    mesh->computeVertexNormals();
    cache->pushData(mesh);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "SUBDIVISIONSURFACE";
    info.cache_proc = subd;
    return info;
}

void unload()
{
}
}
