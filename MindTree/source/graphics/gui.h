#ifndef GUI_5LUHU2WR

#define GUI_5LUHU2WR

#include "boost/python.hpp"
#include "unordered_map"
#include "data/nodes/nodetype.h"

namespace BPy = boost::python;

class NodeDesigner
{
public:
    NodeDesigner(BPy::object designer);
    NodeDesigner(const NodeDesigner &other);
    virtual ~NodeDesigner();
    static void registerDesigner(QString type, NodeDesigner);
    static void removeDesigner(QString type);
    static void exposeToPython();
    static void init();
    //static QGraphicsItem* create(MindTree::NodeType t);
    QGraphicsItem* operator()();


private:
    BPy::object nodeDesigner;
    //static std::unordered_map<MindTree::NodeType, NodeDesigner> designers;
    static bool initialized;
};


#endif /* end of include guard: GUI_5LUHU2WR */
