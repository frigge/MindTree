#ifndef MATERIAL_RES1NWMZ

#define MATERIAL_RES1NWMZ
#include "memory"
#include "data/mtobject.h"

class MaterialInstance;
class Material : public MindTree::Object
{
public:
    Material(std::string name = "Material");
    virtual ~Material();

    std::string getName() const;
    void setName(std::string name);

private:
    friend class MaterialInstance;
    std::string _name;
};

typedef std::shared_ptr<Material> MaterialPtr;

class DefaultMaterial : public Material
{
public:
    DefaultMaterial();
};

class MaterialInstance : public MindTree::Object
{
public:
    MaterialInstance(MaterialPtr material);
    virtual ~MaterialInstance();

    MaterialPtr getMaterial();

private:
    MaterialPtr _material;
    std::string _name;
};

typedef std::shared_ptr<MaterialInstance> MaterialInstancePtr;
#endif /* end of include guard: MATERIAL_RES1NWMZ */
