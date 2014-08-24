#include "mtobject.h"

using namespace MindTree;

Object::Object()
{
}

Object::Object(const Object &other)
{
    for(auto prop : other.getProperties())
        properties[prop.first] = prop.second;
}

const Property Object::getProperty(std::string name)const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    const auto prop = properties[name];
    return prop;
}

Property Object::getProperty(std::string name)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return properties[name];
}

const PropertyMap& Object::getProperties()   const 
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return properties;
}

Property Object::operator[](std::string name)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return properties[name];
}

void Object::setProperty(std::string name, Property value)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    auto prop = properties.find(name);
    if(prop != properties.end()) {
        properties.erase(name);
    }

    properties[name] = value;
}

void Object::rmProperty(std::string name)    
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    properties.erase(name);
}

bool Object::hasProperty(std::string name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return properties.find(name) != properties.end();
}
