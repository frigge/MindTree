#include "mindtree_core.h"
#include "data/cache_main.h"

namespace BPy = boost::python;

bool testSocketProperties()
{
    MindTree::DNode *node = MindTree::NodeDataBase::createNode("Values.Float Value");
    node->getInSockets()[0]->setProperty(2.5);

    MindTree::DataCache *cache = new MindTree::DataCache(node->getOutSockets()[0]);

    std::cout << "cached value is: " << cache->getData(0).getData<double>() << std::endl;
    
    return cache->getData(0).getData<double>() == 2.5;
}

bool testProperties()
{
    bool success=true;
    MindTree::Property floatprop{2.5};
    std::cout << "created " << floatprop.getType() << " Property: "
        << floatprop.getData<double>() << std::endl;
    success = success && floatprop.getData<double>() == 2.5;

    MindTree::Property intprop{2};
    std::cout << "created " << intprop.getType() << " Property: "
        << intprop.getData<int>() << std::endl;
    success = success && intprop.getData<int>() == 2;

    MindTree::Property stringprop{std::string("hallo test")};
    std::cout << "created " << stringprop.getType() << " Property: "
        << stringprop.getData<std::string>() << std::endl;
    success = success && stringprop.getData<std::string>() == "hallo test";

    MindTree::Property colorprop{glm::vec4(1, 0, 0, 1)};
    glm::vec4 color = colorprop.getData<glm::vec4>();
    std::cout << "created " << colorprop.getType() << " Property: ("
        << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;
    success = success && colorprop.getData<glm::vec4>() == glm::vec4(1, 0, 0, 1);

    glm::vec3 oldvec = glm::vec3(24454.456, 10, 25);
    MindTree::Property vec3prop{oldvec};
    glm::vec3 vec3 = vec3prop.getData<glm::vec3>();
    std::cout << "created " << vec3prop.getType() << " Property: ("
        << vec3.x << ", " << vec3.y << ", " << vec3.z << ")" << std::endl;
    success = success && vec3 == oldvec;

    return success;
}

BOOST_PYTHON_MODULE(cpp_tests)
{
    BPy::def("testSocketPropertiesCPP", testSocketProperties);    
    BPy::def("testPropertiesCPP", testProperties);
}
