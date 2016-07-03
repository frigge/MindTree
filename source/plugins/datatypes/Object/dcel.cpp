#include <numeric>

#include "dcel.h"

using namespace MindTree;
using namespace MindTree::dcel;

Edge::Edge(Adapter *adapter) :
    Element(adapter),
    m_next{nullptr},
    m_prev{nullptr},
    m_twin{nullptr},
    m_origin{nullptr},
    m_incidentFace{nullptr}
{
}

Edge* Edge::next()
{
    return m_next;
}

Edge* Edge::prev()
{
    return m_prev;
}

void Edge::setNext(Edge *edge)
{
    assert(m_twin != edge);
    assert(edge != this);
    assert(edge != m_prev);

    if (m_next == edge)
        return;

    m_next = edge;
    if(edge->m_prev != this)
        edge->setPrev(this);

    m_twin->setPrev(edge->m_twin);
}

void Edge::setPrev(Edge *edge)
{
    assert(m_twin != edge);
    assert(edge != this);
    assert(edge != m_next);

    if (m_prev == edge)
        return;

    m_prev = edge;
    if(edge->m_next != this)
        edge->setNext(this);

    m_twin->setNext(edge->m_twin);
}

void Edge::setTwin(Edge *edge)
{
    if (m_twin == edge)
        return;

    m_twin = edge;
    if(edge->m_twin != this)
        edge->m_twin = this;
}

Edge* Edge::twin()
{
    return m_twin;
}

Vertex* Edge::origin()
{
    return m_origin;
}

void Edge::setOrigin(Vertex *vertex)
{
    m_origin = vertex;
    if(!vertex->incident())
        vertex->setIncident(this);
    assert(vertex->incident()->origin() == vertex);
}
void Edge::setIncidentFace(Face *face)
{
    if(m_incidentFace == face)
        return;
    m_incidentFace = face;
    face->setOuterBoundary(this);
    if(m_next)
        m_next->setIncidentFace(face);
}

Face* Edge::incidentFace()
{
    return m_incidentFace;
}

Vertex::Vertex(size_t index, Adapter *adapter) :
    Element(adapter),
    m_incident{nullptr},
    m_index{index}
{
}

Edge* Vertex::incident()
{
    return m_incident;
}

void Vertex::setIncident(Edge *edge)
{
    m_incident = edge;
    if(edge->origin() != this)
        edge->setOrigin(this);
    assert(edge->origin() == this);
}

size_t Vertex::index()
{
    return m_index;
}

void Vertex::setIndex(size_t i)
{
    m_index = i;
}

void Vertex::set(const std::string &name, Property prop)
{
    m_properties[name] = prop;
}

std::vector<Edge*> Vertex::getAdjacentEdges() const
{
    std::vector<Edge*> ret;
    auto *incident = m_incident;
    if(!incident) return ret;

    do {
        ret.push_back(incident);
        if(incident->twin())
            incident = incident->twin()->next();
    } while(incident && incident != m_incident);

    return ret;
}

void Vertex::insertOuter(Edge *edge)
{
    Edge *next{m_incident};
    Edge *prev{nullptr};

    //find outer edge (the one without a face)
    while(next->incidentFace())
        next = next->twin()->next();

    assert(edge->twin() != next);

    if (next == edge)
        return;
    
    if(next)
        prev = next->prev();

    if(prev) {
        edge->setPrev(prev);
    }
    if(next) {
        edge->twin()->setNext(next);
    }

}

void Vertex::insertInner(Edge *edge)
{
    //split face if edge and twin share the same face
    if(edge->incidentFace()
       && edge->incidentFace() == edge->twin()->incidentFace()) {
        auto *face = m_adapter->newFace();
        auto *e = edge;
        do {
            e->setIncidentFace(face);
            e = e->next();
        } while(e != edge);
    }
}

const Property& Vertex::get(const std::string &name) const
{
    return m_properties[name];
}

Property& Vertex::get(const std::string &name)
{
    return m_properties[name];
}

Face::Face(Adapter *adapter) :
    Element(adapter),
    m_outerBoundary{nullptr}
{
}

void Face::setOuterBoundary(Edge *edge)
{
    m_outerBoundary = edge;
}

Edge* Face::outerBoundary()
{
    return m_outerBoundary;
}

namespace {
    class EdgeKey {
        int v0_, v1_;
    public:
        EdgeKey(const int &v0, const int &v1) :
            v0_{v0 < v1 ? v0 : v1},
            v1_{v0 < v1 ? v1 : v0}
        {}

        int v0() const { return v0_; }
        int v1() const { return v1_; }

        bool operator==(const EdgeKey &edge) const
        {
            return edge.v0_ == v0_ && edge.v1_ == v1_;
        }
    };
}

namespace std {
    template<>
    struct hash<EdgeKey> {
        typedef size_t result_type;
        typedef EdgeKey argument_type;

        result_type operator()(argument_type const &value) const
        {
            size_t lhs = value.v0();
            size_t rhs = value.v1();
            return lhs^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
        }
    };
}

Adapter::Adapter(std::shared_ptr<MeshData> mesh) :
    m_mesh(mesh)
{
    auto points = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    auto polygons = mesh->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();

    int i=0;
    for(auto p : *points) {
        m_vertices.push_back(std::make_unique<Vertex>(i, this));
        ++i;
    }

    std::unordered_map<EdgeKey, Edge*> edge_map;
    for(const Polygon &p : *polygons) {
        Edge *prev{nullptr};
        std::vector<EdgeKey> adjacent_edges;
        std::adjacent_find(begin(p),
                           end(p),
                           [&adjacent_edges](const auto &a, const auto &b) {
                               adjacent_edges.push_back(EdgeKey(a, b));
                               return false;
                           });

        for (int i : p) {
            Vertex *origin = m_vertices[i].get();
            m_edges.push_back(std::make_unique<Edge>(this));
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
        m_faces.push_back(std::make_unique<Face>(this));
        auto face = m_faces.back().get();
        face->setOuterBoundary(m_edges.back().get());
    }
}

void Adapter::splitEdge(Edge *edge)
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

Edge* Adapter::insertEdge(Edge *edge)
{
    auto newNext = newEdge();

    edge->setNext(newNext);
    newNext->setNext(edge->next());

    newNext->setIncidentFace(edge->incidentFace());
    newNext->twin()->setIncidentFace(edge->twin()->incidentFace());

    return newNext;
}

Edge* Adapter::newEdge()
{
    auto newEdge = std::make_unique<Edge>(this);
    auto twin = std::make_unique<Edge>(this);
    newEdge->setTwin(twin.get());
    auto *edge = newEdge.get();
    m_edges.push_back(std::move(newEdge));
    m_edges.push_back(std::move(twin));
    return edge;
}

Vertex * Adapter::newVertex()
{
    auto newVertex = std::make_unique<Vertex>(m_vertices.size(), this);
    auto *vertex = newVertex.get();
    m_vertices.push_back(std::move(newVertex));

    for (auto prop : m_mesh->getProperties()) {
        if(prop.second.isList() && prop.second.size() == m_vertices.size())
            Property::setItem(prop.second, prop.second.size(), Property());
    }
    return vertex;
}

Face * Adapter::newFace()
{
    auto newFace = std::make_unique<Face>(this);
    auto *face = newFace.get();
    m_faces.push_back(std::move(newFace));
    return face;
}

Edge* Adapter::connect(Vertex *v1, Vertex *v2)
{
    assert(v1 != v2);
    //find edges that need to be split
    auto *edge = newEdge();

    edge->setOrigin(v1);
    edge->twin()->setOrigin(v2);
    v1->insertOuter(edge);
    v2->insertOuter(edge->twin());

    return edge;
}

std::shared_ptr<MeshData> Adapter::getMesh()
{
    return m_mesh;
}

void Adapter::updateMesh()
{
    //update polygons
    std::shared_ptr<PolygonList> polygons;
    std::shared_ptr<VertexList> vertices;

    if(!m_mesh->hasProperty("polygon")) {
        polygons = std::make_shared<PolygonList>();
        m_mesh->setProperty("polygon", polygons);
    }
    else {
        polygons = m_mesh->getProperty("polygon")
            .getData<std::shared_ptr<PolygonList>>();
        polygons->clear();
        polygons->reserve(m_faces.size());
    }

    if(!m_mesh->hasProperty("P")) {
        vertices = std::make_shared<VertexList>();
        m_mesh->setProperty("P", vertices);
    }
    else {
        vertices = m_mesh->getProperty("P")
            .getData<std::shared_ptr<VertexList>>();
        vertices->clear();
        vertices->reserve(m_vertices.size());
    }

    for(const auto &vert : m_vertices) {
        vertices->push_back(vert->get("P").getData<glm::vec3>());
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

    return;
}

void Adapter::fill(std::initializer_list<Vertex*> vertices)
{
    assert(vertices.size() > 2);
    auto v1 = *vertices.begin();
    auto b = begin(vertices), e = end(vertices);

    //find edge path that contains all vertices
    Edge *edge = v1->incident();
    do {
        auto *nv = edge->twin()->origin();
        auto *tmp = edge;
        bool found = true;
        do {
            if(std::find(b, e, nv) == e) {
                edge = edge->twin()->next();
                found = false;
                break;
            }
            tmp = tmp->next();
        } while(tmp != edge);
        if (found)
            break;
    } while (edge != v1->incident());

    //recursively assigns face to whole edge loop
    edge->setIncidentFace(newFace());
}

void Adapter::remove(Edge *edge)
{
}

void Adapter::collapse(Edge *edge)
{
}
