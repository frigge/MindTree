#include "mtobject.h"

using namespace MindTree;

Object::Object()
{
}

Object::Object(const Object &other)
{
    for(auto prop : other.getProperties())
        _properties[prop.first] = prop.second;
}

Object::Object(const Object &&other) : 
    _properties{std::move(other._properties)}

{
    for(auto prop : other.getProperties())
        _properties[prop.first] = prop.second;
}

Object::~Object()
{
}

Object& Object::operator=(const Object &other)
{
    _properties = other._properties;
    return *this;
}

Object& Object::operator=(const Object &&other)
{
    _properties = std::move(other._properties);
    return *this;
}

Property Object::getProperty(std::string name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    auto prop = _properties.at(name);
    return prop;
}

const PropertyMap& Object::getProperties() const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return _properties;
}

Property Object::operator[](std::string name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return _properties.at(name);
}

void Object::setProperty(std::string name, Property value)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    auto prop = _properties.find(name);
    if(prop != _properties.end()) {
        _properties.erase(name);
    }

    _properties[name] = value;
}

void Object::rmProperty(std::string name)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    _properties.erase(name);
}

bool Object::hasProperty(std::string name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return _properties.find(name) != _properties.end();
}
