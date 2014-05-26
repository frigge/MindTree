#ifndef MTOBJECT_H
#define MTOBJECT_H

#include "mutex"
#include "data/properties.h"

namespace MindTree {

class Object {
public:
    Object();
    Object(const Object &other);
    ~Object();

    Property getProperty(std::string name);
    const Property getProperty(std::string name)const;
    const PropertyMap& getProperties()const;
    void setProperty(std::string, Property value);
    Property operator[](std::string name);
    void rmProperty(std::string name);
    bool hasProperty(std::string name) const;

private:
    mutable PropertyMap properties;
    mutable std::mutex _propertiesLock;
};
}
#endif
