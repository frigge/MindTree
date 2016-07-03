#ifndef MT_OBJECT_DCEL_H
#define MT_OBJECT_DCEL_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "./object.h"

namespace MindTree {
namespace dcel {

class Adapter;
class Element
{
public:
    Element(Adapter *adapter): m_adapter(adapter) {}
protected:
    Adapter *m_adapter;
};

class Vertex;
class Face;
 class Edge : public Element
{
public:
    Edge(Adapter *adapter);

    Edge* next();
    Edge* prev();
    Edge* twin();

    void setNext(Edge *edge);
    void setPrev(Edge *edge);
    void setTwin(Edge *edge);

    void setOrigin(Vertex *vertex);
    Vertex* origin();

    void setIncidentFace(Face *face);
    Face* incidentFace();

private:
    Edge *m_next;
    Edge *m_prev;
    Edge *m_twin;
    Vertex *m_origin;
    Face *m_incidentFace;
};

 class Vertex : public Element
{
public:
    Vertex(size_t index, Adapter *adapter);

    Edge* incident();
    void setIncident(Edge *edge);
    size_t index();
    void setIndex(size_t i);
    void set(const std::string &name, Property prop);
    const Property& get(const std::string &name) const;
    Property& get(const std::string &name);
    std::vector<Edge*> getAdjacentEdges() const;
    void insertOuter(Edge *edge);
    void insertInner(Edge *edge);

private:
    Edge *m_incident;
    size_t m_index;
    PropertyMap m_properties;
};

 class Face : public Element
{
public:
    Face(Adapter *adapter);

    void setOuterBoundary(Edge *edge);
    Edge* outerBoundary();

private:
    Edge *m_outerBoundary;
    //std::vector<Edge*> m_innerBoundaries;
};

class Adapter
{
public:
    Adapter(std::shared_ptr<MeshData> mesh);

    std::vector<Edge*> getAdjacentEdges(Vertex *vertex);
    void splitEdge(Edge *edge);
    Edge* connect(Vertex *v1, Vertex *v2);
    void remove(Edge *edge);
    void collapse(Edge *edge);
    Vertex* newVertex();
    void fill(std::initializer_list<Vertex*> vertices);

    std::shared_ptr<MeshData> getMesh();
    void updateMesh();
    Face * newFace();

private:
    Edge* newEdge();
    Edge* insertEdge(Edge *edge);

    std::shared_ptr<MeshData> m_mesh;
    std::vector<std::unique_ptr<Edge>> m_edges;
    std::vector<std::unique_ptr<Vertex>> m_vertices;
    std::vector<std::unique_ptr<Face>> m_faces;
};
}
}

#endif
