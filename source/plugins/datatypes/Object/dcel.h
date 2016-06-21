#ifndef MT_OBJECT_DCEL_H
#define MT_OBJECT_DCEL_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "./object.h"

namespace MindTree {
class HalfEdgeVertex;
class HalfEdgeFace;
class HalfEdge
{
public:
    HalfEdge();

    HalfEdge* next();
    HalfEdge* prev();
    HalfEdge* twin();

    void setNext(HalfEdge *edge);
    void setPrev(HalfEdge *edge);
    void setTwin(HalfEdge *edge);

    void setOrigin(HalfEdgeVertex *vertex);
    HalfEdgeVertex* origin();

    void setIncidentFace(HalfEdgeFace *face);
    HalfEdgeFace* incidentFace();

private:
    HalfEdge *m_next;
    HalfEdge *m_prev;
    HalfEdge *m_twin;
    HalfEdgeVertex *m_origin;
    HalfEdgeFace *m_incidentFace;
};

class HalfEdgeVertex
{
public:
    HalfEdgeVertex(size_t index);

    HalfEdge* incident();
    void setIncident(HalfEdge *edge);
    size_t index();
    void setIndex(size_t i);

private:
    HalfEdge *m_incident;
    size_t m_index;
};

class HalfEdgeFace
{
public:
    HalfEdgeFace();

    void setOuterBoundary(HalfEdge *edge);
    HalfEdge* outerBoundary();

private:
    HalfEdge *m_outerBoundary;
    //std::vector<HalfEdge*> m_innerBoundaries;
};

class HalfEdgeAdapter
{
public:
    HalfEdgeAdapter(std::shared_ptr<MeshData> mesh);

    std::vector<HalfEdge*> getAdjacentEdges(HalfEdgeVertex *vertex);
    void splitEdge(HalfEdge *edge);
    void connect(HalfEdgeVertex *v1, HalfEdgeVertex *v2);
    void remove(HalfEdge *edge);
    void collapse(HalfEdge *edge);

    std::shared_ptr<MeshData> getMesh();

private:
    HalfEdge* newEdge();
    HalfEdgeVertex * newVertex();
    HalfEdgeFace * newFace();
    HalfEdge* insertEdge(HalfEdge *edge);

    std::shared_ptr<MeshData> m_mesh;
    std::vector<std::unique_ptr<HalfEdge>> m_edges;
    std::vector<std::unique_ptr<HalfEdgeVertex>> m_vertices;
    std::vector<std::unique_ptr<HalfEdgeFace>> m_faces;
};
}

#endif
