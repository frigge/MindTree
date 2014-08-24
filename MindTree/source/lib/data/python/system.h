#ifndef MT_PYTHON_SYSTEM_WRAPS_H
#define MT_PYTHON_SYSTEM_WRAPS

#include "boost/python.hpp"

namespace BPy = boost::python;

namespace MindTree {
namespace Python {

namespace sys {
    void wrap();

    void open(std::string filename);
    void newProject();
    void regNode(PyObject *nodeClass);
    BPy::object createNode(std::string name);
    BPy::list getRegisteredNodes();
    Signal::CallbackHandler attachToSignal(std::string id, BPy::object fn);
    Signal::CallbackHandler attachToBoundSignal(BPy::object livetime, std::string id, BPy::object fn);
    std::vector<std::string> getNodeTypes();
    std::string __str__StringVector(std::vector<std::string> &self);
    std::string __repr__StringVector(std::vector<std::string> &self);
    std::vector<std::string> getSocketTypes();
}
}
}
#endif
