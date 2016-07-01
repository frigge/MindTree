#include <stdexcept>
#include "mtobject.h"

using namespace MindTree;

Object::Object()
{
}

Object::Object(const Object &other)
    : _properties(other._properties)
{
}

Object::Object(const Object &&other) : 
    _properties{std::move(other._properties)}

{
}

Object::~Object()
{
}

Object& Object::operator=(const Object &other)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    std::lock_guard<std::mutex> lock2(other._propertiesLock);

    _properties = other._properties;
    return *this;
}

Object& Object::operator=(const Object &&other)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    std::lock_guard<std::mutex> lock2(other._propertiesLock);

    _properties = std::move(other._properties);
    return *this;
}

const Property& Object::getProperty(const std::string &name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    if (_properties.find(name) == end(_properties)) {
        throw std::runtime_error("no property named " + name + " found");
        assert(false);
    }
    return _properties.at(name);
}

Property& Object::getProperty(const std::string &name)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    if (_properties.find(name) == end(_properties)) {
        throw std::runtime_error("no property named " + name + " found");
        assert(false);
    }
    return _properties[name];
}

PropertyMap Object::getProperties() const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return _properties;
}

Property Object::operator[](const std::string &name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    if (_properties.find(name) == end(_properties))
        return Property();
    return _properties.at(name);
}

void Object::setProperty(const std::string &name, const Property &value)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    _properties[name] = value;
}

void Object::rmProperty(const std::string &name)
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    if (_properties.find(name) == end(_properties))
        return;
    _properties.erase(name);
}

bool Object::hasProperty(const std::string &name) const
{
    std::lock_guard<std::mutex> lock(_propertiesLock);
    return _properties.find(name) != _properties.cend();
}
