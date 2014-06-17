#include "material.h"

Material::Material(std::string name)
    : _name(name)
{
}

Material::~Material()
{
}

std::string Material::getName() const
{
    return _name;
}

void Material::setName(std::string name)
{
    _name = name;
}

MaterialInstance::MaterialInstance(MaterialPtr material)
    : _material(material)
{
}

MaterialInstance::~MaterialInstance()
{
}

MaterialPtr MaterialInstance::getMaterial()
{
    return _material;
}
