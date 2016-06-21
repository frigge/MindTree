#include <numeric>

#include "dcel.h"

using namespace MindTree;

HalfEdge::HalfEdge() :
    m_next{nullptr},
    m_prev{nullptr},
    m_twin{nullptr},
    m_origin{nullptr},
    m_incidentFace{nullptr}
{
}

HalfEdge* HalfEdge::next()
{
    return m_next;
}

HalfEdge* HalfEdge::prev()
{
    return m_next;
}

void HalfEdge::setNext(HalfEdge *edge)
{
    if (m_next == edge)
        return;

    m_next = edge;
    if(edge->m_prev != this)
        edge->m_prev = this;
    if(m_twin && edge->m_twin)
        m_twin->setPrev(edge->m_twin);
}

void HalfEdge::setPrev(HalfEdge *edge)
{
    if (m_prev == edge)
        return;

    m_prev = edge;
    if(edge->m_next != this)
        edge->m_next = this;
    if(m_twin && edge->m_twin)
        m_twin->setNext(edge->m_twin);
}

void HalfEdge::setTwin(HalfEdge *edge)
{
    if (m_twin == edge)
        return;

    m_twin = edge;
    if(edge->m_twin != this)
        edge->m_twin = this;
}

HalfEdge* HalfEdge::twin()
{
    return m_next;
}

HalfEdgeVertex* HalfEdge::origin()
{
    return m_origin;
}

void HalfEdge::setOrigin(HalfEdgeVertex *vertex)
{
    m_origin = vertex;
}
void HalfEdge::setIncidentFace(HalfEdgeFace *face)
{
    m_incidentFace = face;
}

HalfEdgeFace* HalfEdge::incidentFace()
{
    return m_incidentFace;
}

HalfEdgeVertex::HalfEdgeVertex(size_t index) :
    m_incident{nullptr},
    m_index{index}
{
}

HalfEdge* HalfEdgeVertex::incident()
{
    return m_incident;
}

void HalfEdgeVertex::setIncident(HalfEdge *edge)
{
    m_incident = edge;
}

size_t HalfEdgeVertex::index()
{
    return m_index;
}

void HalfEdgeVertex::setIndex(size_t i)
{
    m_index = i;
}

HalfEdgeFace::HalfEdgeFace() :
    m_outerBoundary{nullptr}
{
}

void HalfEdgeFace::setOuterBoundary(HalfEdge *edge)
{
    m_outerBoundary = edge;
}

HalfEdge* HalfEdgeFace::outerBoundary()
{
    return m_outerBoundary;
}

namespace {
    class Edge {
        int v0_, v1_;
    public:
        Edge(const int &v0, const int &v1) :
            v0_{v0 < v1 ? v0 : v1},
            v1_{v0 < v1 ? v1 : v0}
        {}

        int v0() const { return v0_; }
        int v1() const { return v1_; }

        bool operator==(const Edge &edge) const
        {
            return edge.v0_ == v0_ && edge.v1_ == v1_;
        }
    };
}

namespace std {
    template<>
    struct hash<Edge> {
        typedef size_t result_type;
        typedef Edge argument_type;

        result_type operator()(argument_type const &value) const
        {
            size_t lhs = value.v0();
            size_t rhs = value.v1();
            return lhs^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
        }
    };
}

HalfEdgeAdapter::HalfEdgeAdapter(std::shared_ptr<MeshData> mesh) :
    m_mesh(mesh)
{
    auto points = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    auto polygons = mesh->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();

    int i=0;
    for(auto p : *points) {
        m_vertices.push_back(std::make_unique<HalfEdgeVertex>(i));
        ++i;
    }

    std::unordered_map<Edge, HalfEdge*> edge_map;
    for(const Polygon &p : *polygons) {
        HalfEdge *prev{nullptr};
        std::vector<Edge> adjacent_edges;
        std::adjacent_find(begin(p),
                           end(p),
                           [&adjacent_edges](const auto &a, const auto &b) {
                               adjacent_edges.push_back(Edge(a, b));
                               return false;
                           });

        int i = 0;
        for (int i : p) {
            HalfEdgeVertex *origin = m_vertices[i].get();
            m_edges.push_back(std::make_unique<HalfEdge>());
            auto newEdge = m_edges.back().get();
            if(!origin->incident()) origin->setIncident(newEdge);

            if(edge_map.find(adjacent_edges[i]) != end(edge_map))
                newEdge->setTwin(edge_map[adjacent_edges[i]]);
            else
                edge_map[adjacent_edges[i]] = newEdge;

            if(prev) newEdge->setPrev(prev);
            prev = newEdge;
            newEdge->setOrigin(origin);
            ++i;
        }
        m_faces.push_back(std::make_unique<HalfEdgeFace>());
        auto face = m_faces.back().get();
        face->setOuterBoundary(m_edges.back().get());
    }
}

std::vector<HalfEdge*> HalfEdgeAdapter::getAdjacentEdges(HalfEdgeVertex *vertex)
{
    std::vector<HalfEdge*> ret;
    auto *incident = vertex->incident();
    ret.push_back(incident);
    while((incident = incident->twin()->next()))
        ret.push_back(incident);

    return ret;
}

void HalfEdgeAdapter::splitEdge(HalfEdge *edge)
{
    auto newNext = insertEdge(edge);
    auto newTwin = newNext->twin();

    auto newOrigin = newVertex();
    newNext->twin()->setOrigin(edge->twin()->origin());

    auto verts = m_mesh->getProperty("P").getData<VertexListPtr>();
    glm::vec3 prevPos = verts->at(edge->origin()->index());
    glm::vec3 nextPos = verts->at(edge->twin()->origin()->index());

    glm::vec3 newPos = prevPos + ((prevPos - nextPos) * 0.5f);
    (*verts)[newOrigin->index()] = newPos;

    newNext->setOrigin(newOrigin);
    edge->twin()->setOrigin(newOrigin);
}

HalfEdge* HalfEdgeAdapter::insertEdge(HalfEdge *edge)
{
    auto newNext = newEdge();

    edge->setNext(newNext);
    newNext->setNext(edge->next());

    newNext->setIncidentFace(edge->incidentFace());
    newNext->twin()->setIncidentFace(edge->twin()->incidentFace());

    return newNext;
}

HalfEdge* HalfEdgeAdapter::newEdge()
{
    auto newEdge = std::make_unique<HalfEdge>();
    auto twin = std::make_unique<HalfEdge>();
    newEdge->setTwin(twin.get());
    auto *edge = newEdge.get();
    m_edges.push_back(std::move(newEdge));
    m_edges.push_back(std::move(twin));
    return edge;
}

HalfEdgeVertex * HalfEdgeAdapter::newVertex()
{
    auto newVertex = std::make_unique<HalfEdgeVertex>(m_vertices.size());
    auto *vertex = newVertex.get();
    m_vertices.push_back(std::move(newVertex));

    for (auto prop : m_mesh->getProperties()) {
        if(prop.second.isList() && prop.second.size() == m_vertices.size())
            Property::setItem(prop.second, prop.second.size(), Property());
    }
    return vertex;
}

HalfEdgeFace * HalfEdgeAdapter::newFace()
{
    auto newFace = std::make_unique<HalfEdgeFace>();
    auto *face = newFace.get();
    m_faces.push_back(std::move(newFace));
    return face;
}

void HalfEdgeAdapter::connect(HalfEdgeVertex *v1, HalfEdgeVertex *v2)
{
    //find edges that need to be split
    auto adj1 = getAdjacentEdges(v1);
    auto adj2 = getAdjacentEdges(v1);

    HalfEdge *e1 = nullptr, *e2 = nullptr;
    for (auto edge1 : adj1) {
        for (auto edge2 : adj2) {
            if (edge1->incidentFace() == edge2->incidentFace()) {
                e1 = edge1;
                e2 = edge2;
                break;
            }
        }
        if (e1 != nullptr) break;
    }

    //split edges and their twins
    auto *edge = newEdge();

    e1->setPrev(edge);
    edge->setOrigin(v2);
    e2->setPrev(edge->twin());
    edge->setOrigin(v1);

    auto *face = newFace();
    edge->setIncidentFace(e1->incidentFace());
    edge->twin()->setIncidentFace(face);
    for(HalfEdge *e = edge->twin()->next(); e != edge->twin(); e = e->next())
        e->setIncidentFace(face);
}

std::shared_ptr<MeshData> HalfEdgeAdapter::getMesh()
{
    //update polygons
    std::shared_ptr<PolygonList> polygons;
    if(!m_mesh->hasProperty("polygon")) {
        polygons = std::make_shared<PolygonList>();
        m_mesh->setProperty("polygon", polygons);
    }
    else {
        polygons = m_mesh->getProperty("polygons")
            .getData<std::shared_ptr<PolygonList>>();
        polygons->clear();
    }

    for(const auto &face : m_faces) {
        auto *start = face->outerBoundary();
        auto *e = start;
        Polygon p;
        do {
            p.push_back(e->origin()->index());
        } while ((e = e->next()) != start);
        polygons->push_back(p);
    }
    m_mesh->computeVertexNormals();

    return m_mesh;
}

void HalfEdgeAdapter::remove(HalfEdge *edge)
{
}

void HalfEdgeAdapter::collapse(HalfEdge *edge)
{
}
