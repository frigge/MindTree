#include "mindtree_core.h"
#include "../datatypes/Object/object.h"
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
    std::cout << "created " << floatprop.getType().toStr() << " Property: "
        << floatprop.getData<double>() << std::endl;
    success = success && floatprop.getData<double>() == 2.5;

    MindTree::Property intprop{2};
    std::cout << "created " << intprop.getType().toStr() << " Property: "
        << intprop.getData<int>() << std::endl;
    success = success && intprop.getData<int>() == 2;

    MindTree::Property stringprop{std::string("hallo test")};
    std::cout << "created " << stringprop.getType().toStr() << " Property: "
        << stringprop.getData<std::string>() << std::endl;
    success = success && stringprop.getData<std::string>() == "hallo test";

    MindTree::Property colorprop{glm::vec4(1, 0, 0, 1)};
    glm::vec4 color = colorprop.getData<glm::vec4>();
    std::cout << "created " << colorprop.getType().toStr() << " Property: ("
        << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;
    success = success && colorprop.getData<glm::vec4>() == glm::vec4(1, 0, 0, 1);

    glm::vec3 oldvec = glm::vec3(24454.456, 10, 25);
    MindTree::Property vec3prop{oldvec};
    glm::vec3 vec3 = vec3prop.getData<glm::vec3>();
    std::cout << "created " << vec3prop.getType().toStr() << " Property: ("
        << vec3.x << ", " << vec3.y << ", " << vec3.z << ")" << std::endl;
    success = success && vec3 == oldvec;

    return success;
}

bool testPropertiesTypeInfo()
{
    MindTree::Property prop{2.5};
    std::string type1 = prop.getType().toStr();
    std::cout << "original Prop Type: " << type1 << std::endl;
    if(type1 != "FLOAT") return false;

    MindTree::Property copyProp(prop);
    std::cout << "copy Prop Type: " << copyProp.getType().toStr() << std::endl;
    if(type1 != copyProp.getType().toStr())
        return false;

    MindTree::Property assignmentProp;
    assignmentProp = prop;
    std::cout << "assignment Prop Type: " << assignmentProp.getType().toStr() << std::endl;
    if(type1 != assignmentProp.getType().toStr())
        return false;

    return true;
}

bool testObjectInProperty()
{
    GeoObjectPtr obj = std::make_shared<GeoObject>();
    MindTree::Property objProp{obj};
    std::cout << "original Prop Type: " << objProp.getType().toStr() << std::endl;
    if(objProp.getType() != "SCENEOBJECT")
        return false;

    return true;
}

bool testPropertyConversion()
{
    int a = 25;
    std::cout << "creating int value: " << a << std::endl;
    MindTree::Property intprop{a};
    std::cout << "Property has type: " << intprop.getType().toStr() << std::endl;
    double converted = intprop.getData<double>();

    std::cout << "converted to double the resulting value is: " << converted << std::endl;

    int b = converted;
    return a == b;
}

BOOST_PYTHON_MODULE(cpp_tests)
{
    BPy::def("testSocketPropertiesCPP", testSocketProperties);    
    BPy::def("testPropertiesCPP", testProperties);
    BPy::def("testPropertiesTypeInfoCPP", testPropertiesTypeInfo);
    BPy::def("testObjectInPropertyCPP", testObjectInProperty);
    BPy::def("testPropertyConversionCPP", testPropertyConversion);
}
