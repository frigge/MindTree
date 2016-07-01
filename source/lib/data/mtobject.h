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

    const Property& getProperty(const std::string &name) const;
    Property& getProperty(const std::string &name);
    Property operator[](const std::string &name) const;

    PropertyMap getProperties()const;
    virtual void setProperty(const std::string&, const Property &value);
    void rmProperty(const std::string &name);
    bool hasProperty(const std::string &name) const;

private:
    PropertyMap _properties;
    mutable std::mutex _propertiesLock;
};
}
#endif
