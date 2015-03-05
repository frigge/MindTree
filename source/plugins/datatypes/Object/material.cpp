#include "material.h"

PROPERTY_TYPE_INFO(MaterialInstancePtr, "MATERIALINSTANCE");
PROPERTY_TYPE_INFO(MaterialPtr, "MATERIAL");
PROPERTY_TYPE_INFO(std::shared_ptr<DefaultMaterial>, "MATERIAL");

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

DefaultMaterial::DefaultMaterial()
    : Material("Default")
{
    setProperty("specular_intensity", 0.4);
    setProperty("diffuse_intensity", 0.4);
    setProperty("diffuse_color", glm::vec4(1));
}

MaterialInstance::MaterialInstance(MaterialPtr material)
    : _material(material)
{
    for (auto prop : material->getProperties())
        setProperty(prop.first, prop.second);
}

MaterialInstance::~MaterialInstance()
{
}

MaterialPtr MaterialInstance::getMaterial()
{
    return _material;
}
