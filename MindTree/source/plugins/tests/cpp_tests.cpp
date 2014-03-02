#include "mindtree_core.h"
#include "data/cache_main.h"

namespace BPy = boost::python;

bool testSocketProperties()
{
    MindTree::DNode *node = MindTree::NodeDataBase::createNode("Values.Float Value");
    node->getInSockets()[0]->setProperty(2.5);

    MindTree::DataCache *cache = new MindTree::DataCache(node->getOutSockets()[0]);

    std::cout << "cached value is: " << cache->data.getData<double>() << std::endl;
    
    return cache->data.getData<double>() == 2.5;
}

bool testProperties()
{
    MindTree::Property prop{2.5};
    std::cout << "created Property with type: " << prop.getType() << std::endl;
    return prop.getData<double>() == 2.5;
}

BOOST_PYTHON_MODULE(cpp_tests)
{
    BPy::def("testSocketPropertiesCPP", testSocketProperties);    
    BPy::def("testPropertiesCPP", testProperties);
}
