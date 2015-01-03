#ifndef PYNODE_AFSOCJ2U

#define PYNODE_AFSOCJ2U

#include "boost/python.hpp"
#include "dataa/nodes/data_node.h"

namespace BPy = boost::python;

namespace MindTree
{
    
class PyNodeWrapper : public DNode
{
public:
    PyNodeWrapper();
    virtual ~PyNodeWrapper();
    void wrap();

private:
    BPy::object pynode;
};

} /* MindTree */

#endif /* end of include guard: PYNODE_AFSOCJ2U */
