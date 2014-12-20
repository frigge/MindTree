#ifndef MTOBJECT_H
#define MTOBJECT_H

#include "mutex"
#include "data/properties.h"

namespace MindTree {

class Object {
public:
    Object();
    Object(const Object &other);
    Object(const Object &&other);
    virtual ~Object();

    Object& operator=(const Object &other);
    Object& operator=(const Object &&other);

    Property getProperty(std::string name)const;
    Property operator[](std::string name) const;

    const PropertyMap& getProperties()const;
    void setProperty(std::string, Property value);
    void rmProperty(std::string name);
    bool hasProperty(std::string name) const;

private:
    PropertyMap _properties;
    mutable std::mutex _propertiesLock;
};
}
#endif
