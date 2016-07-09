#define GLM_SWIZZLE
#include <set>
#include <unordered_set>

#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "../plugins/datatypes/Object/skeleton.h"
#include "data/reloadable_plugin.h"
#include "common.h"

using namespace MindTree;

void grahamScanMerge(PolygonList *polys)
{
}

void inheritAttributes(PropertyMap *attributes, Joint *j, uint i)
{
    for(const auto &prop : j->getProperties()){
        Property::setItem((*attributes)[prop.first], i, prop.second);
    }
}

std::shared_ptr<MeshData> meshJoint(JointPtr root, uint sides, bool merge_joints)
{
    //differentiate paths and joints
    std::stack<Joint*> stack;
    stack.push(root.get());
    using PointSet = std::unordered_set<uint>;
    using Ring  = std::vector<MeshData::Edge>;
    using JointRings = std::vector<Ring>;
    std::unordered_map<Joint*, JointRings> joint_rings;

    auto mesh = std::make_shared<MeshData>();
    auto points = std::make_shared<VertexList>();
    auto polys = std::make_shared<PolygonList>();
    PropertyMap attributes;
    mesh->setProperty("P", points);
    mesh->setProperty("polygon", polys);

    while(!stack.empty()) {
        auto *joint = stack.top();
        auto trans = joint->getWorldTransformation();
        stack.pop();

        auto children = joint->getChildren();

        if(joint->getParent()) {
            auto currentPos = trans[3].xyz();
            if(currentPos == glm::vec3(0))
                continue;
        }

        Ring lastring;
        //path
        if(children.size() == 1) {
            auto *j = joint;
            Joint *lastjoint{nullptr};
            if(j->getParent() && merge_joints) {
                auto childTrans = children[0]->getWorldTransformation();
                trans = (childTrans + trans) * 0.5f;
                trans[3].w = 1;
            }

            bool first{true};
            while(children.size() == 1) {
                children = j->getChildren();

                auto ring = createRing(sides, points.get());
                if(first && merge_joints) {
                    Joint *key = j;
                    while(key->getParent() &&key->getTransformation()[3].xyz() == glm::vec3(0))
                        key = static_cast<Joint*>(key->getParent());
                    joint_rings[key].push_back(ring);
                }

                first = false;

                //cap start
                if(!j->getParent()) {
                    Polygon p;
                    for(int i = sides-1; i >= 0; --i) {
                        p.push_back(i);
                    }
                    inheritAttributes(&attributes, j, polys->size());
                    polys->push_back(p);
                }

                for(int i = points->size() - sides; i < points->size(); i++) {
                    auto v0 = points->at(i);
                    points->at(i) = (trans * glm::vec4(v0, 1)).xyz();
                }

                if(!lastring.empty()) {
                    for(int i = 0; i < ring.size(); ++i) {
                        Polygon p{ring[i].v1(), ring[i].v0(),
                                lastring[i].v0(), lastring[i].v1()};
                        inheritAttributes(&attributes, j, polys->size());
                        polys->push_back(p);
                    }
                }
                lastring = ring;

                if(children.empty() || children[0]->getType() != AbstractTransformable::JOINT) {
                    break;
                }

                lastjoint = j;
                if(!children.empty()) {
                    j = std::static_pointer_cast<Joint>(children[0]).get();
                    trans = j->getWorldTransformation();
                }
            }
            if(!children.empty()) {
                joint_rings[lastjoint] = JointRings();
                joint_rings[lastjoint].push_back(lastring);
            }
        }

        for(const auto &child : children) {
            if(child->getType() != AbstractTransformable::JOINT)
                continue;

            stack.push(std::static_pointer_cast<Joint>(child).get());
        }

        //cap end
        if(children.empty()) {
            Polygon p;
            for(int i = points->size() - sides; i < points->size(); i++) {
                p.push_back(i);
            }

            inheritAttributes(&attributes, joint, polys->size());
            polys->push_back(p);
        }
    }

    if(merge_joints) {
        std::unordered_map<MeshData::Edge, uint> edge_map;
        for(auto &joints : joint_rings) {
            PointSet joint_points;
            for(auto &ring : joints.second) {
                for(const auto &edge : ring) {
                    joint_points.insert(edge.v0());
                    joint_points.insert(edge.v1());

                    edge_map[edge] = 1;
                }
            }

            std::unordered_set<MeshData::Edge> processed_edges;
            while(!edge_map.empty()
                  && !joint_points.empty()) {
                auto &first = edge_map.begin()->first;
                Polygon poly{first.v1(), first.v0()};
                glm::vec3 v0 = points->at(first.v1());
                glm::vec3 v1 = points->at(first.v0());

                //try a triangle
                int best_match = -1;
                for(uint p : joint_points) {
                    if(p == first.v0() || p == first.v1())
                        continue;

                    auto end = processed_edges.end();
                    if(processed_edges.find(MeshData::Edge(first.v0(), p)) != end
                       || processed_edges.find(MeshData::Edge(first.v1(), p)) != end)
                        continue;

                    if(best_match == -1) {
                        best_match = p;
                        continue;
                    }
                    
                    glm::vec3 best = points->at(best_match);
                    glm::vec3 pos = points->at(p);
                    glm::vec3 center = (v0 + v1 + best) / 3.0f;
                    auto n = glm::cross(v1 - v0, best - v0);
                    if(glm::dot(n, pos - center) > 0) {
                        best_match = p;
                    }
                }
                if(best_match != -1) {
                    poly.push_back(best_match);

                    MeshData::Edge e1(poly[1], poly[2]);
                    MeshData::Edge e2(poly[2], poly[0]);

                    processed_edges.insert(first);
                    edge_map.erase(first);
                    for(const auto &e : {e1, e2}) {
                        auto it = edge_map.find(e);
                        if(it != edge_map.end()) {
                            it->second++;
                            if(it->second == 2) {
                                edge_map.erase(it);
                                processed_edges.insert(e);
                            }
                        }
                        else {
                            edge_map[e] = 1;
                        }
                    }

                    inheritAttributes(&attributes, joints.first, polys->size());
                    polys->push_back(poly);
                }
                else {
                    processed_edges.insert(edge_map.begin()->first);
                    edge_map.erase(edge_map.begin());
                }
                
                std::vector<uint> to_be_killed;
                for (auto it = joint_points.begin(); it != joint_points.end(); ++it) {
                    auto eit = std::find_if(edge_map.begin(),
                                            edge_map.end(),
                                            [&it](const auto &edge) {
                                                return edge.first.v0() == *it
                                                || edge.first.v1() == *it;
                        });

                    if(eit == edge_map.end())
                        to_be_killed.push_back(*it);
                }

                for(auto i : to_be_killed)
                    joint_points.erase(i);
            }
        }
    }

    mesh->computeVertexNormals();
    return mesh;
}

std::shared_ptr<MeshData> meshJointsDCEL(JointPtr root)
{
    static const int SIDES = 3;
    //differentiate paths and joints
    std::stack<Joint*> stack;
    stack.push(root.get());
    std::vector<Joint*> joints;
    std::vector<std::pair<Joint*, Joint*>> paths;

    auto mesh = std::make_shared<MeshData>();
    auto points = std::make_shared<VertexList>();
    auto polys = std::make_shared<PolygonList>();
    mesh->setProperty("P", points);
    mesh->setProperty("polygon", polys);
    dcel::Adapter adapter(mesh);

    while(!stack.empty()) {
        auto *joint = stack.top();
        auto trans = joint->getWorldTransformation();
        stack.pop();

        auto children = joint->getChildren();

        //path
        if(children.size() == 1) {
            auto *j = joint;
            dcel::Edge *lastring{nullptr};
            while(children.size() == 1) {
                auto *ring = createRingDCEL(SIDES, &adapter);
                auto *edge = ring;
                do {
                    auto p = edge->origin()->get("P").getData<glm::vec3>();
                    p = (trans * glm::vec4(p, 1)).xyz();
                    edge->origin()->set("P", p);
                    edge = edge->next();
                } while (edge != ring);

                if(lastring) {
                    auto *e1 = lastring;
                    auto *e2 = ring;

                    do {
                        adapter.connect(e1->origin(), e2->origin());
                        if(e1 != lastring) {
                            adapter.fill({e1->origin(),
                                        e2->origin(),
                                        e2->prev()->origin(),
                                        e1->prev()->origin()});
                        }
                        e1 = e1->next();
                        e2 = e2->next();
                    } while(e1 != lastring);
                }
                lastring = ring;

                if(children[0]->getType() != AbstractTransformable::JOINT) {
                    break;
                }

                j = std::static_pointer_cast<Joint>(children[0]).get();
                children = j->getChildren();
            }
        }
        for(const auto &child : children) {
            if(child->getType() != AbstractTransformable::JOINT)
                continue;

            stack.push(std::static_pointer_cast<Joint>(child).get());
        }
    }

    adapter.updateMesh();
    return mesh;
}

void meshing(DataCache* cache)
{
    auto joints = cache->getData(0).getData<JointPtr>();
    bool merge = cache->getData(1).getData<bool>();
    auto sides = cache->getData(2).getData<int>();
    cache->pushData(meshJoint(joints, sides, merge)); 
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "MESHING";
    info.cache_proc = meshing;
    return info;
}

void unload()
{
}
}
