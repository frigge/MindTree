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

Object::~Object()
{
}

const Property Object::getProperty(std::string name)const
{
    const auto prop = properties[name];
    return prop;
}

Property Object::getProperty(std::string name)
{
    return properties[name];
}

const PropertyMap& Object::getProperties()   const 
{
    return properties;
}

Property Object::operator[](std::string name)
{
    return properties[name];
}

void Object::setProperty(std::string name, Property value)
{
    auto prop = properties.find(name);
    if(prop != properties.end()) {
        properties.erase(name);
    }

    properties[name] = value;
}

void Object::rmProperty(std::string name)    
{
    properties.erase(name);
}

bool Object::hasProperty(std::string name) const
{
    return properties.find(name) != properties.end();
}
