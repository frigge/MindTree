#include "material.h"

std::unordered_map<std::string, Material*> Material::materials;

Material::Material(std::string name)
{
    auto idname = name;
    int id = 1;
    while(materials.count(idname))
        idname = name + std::to_string(id++);
    this->name = idname;
    materials.insert(std::make_pair(idname, this));
}

Material::~Material()
{
   materials.erase(name); 
}

void Material::setProp(std::string name, MindTree::Property prop)    
{
    properties[name] = prop;
}

std::string Material::getName() const
{
    return name;
}

std::unordered_map<std::string, Material*> Material::getMaterials()    
{
    return materials;
}
