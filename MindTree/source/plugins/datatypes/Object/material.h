#ifndef MATERIAL_RES1NWMZ

#define MATERIAL_RES1NWMZ
#include "data/properties.h"

class Material
{
public:
    Material(std::string name = "");
    virtual ~Material();

    template<typename T>
    T getProp(std::string name) {
        try {
            return properties[name].getData<T>();
        } catch (const std::out_of_range &e){ 
            std::cout << "no Property of name "<< name << " stored" << std::endl;
            return T();
        }
    }

    void setProp(std::string name, MindTree::Property prop);
    std::string getName() const;
    static std::unordered_map<std::string, Material*> getMaterials();

private:
    static std::unordered_map<std::string, Material*> materials;
    MindTree::PropertyMap properties;
    std::string name;
};

#endif /* end of include guard: MATERIAL_RES1NWMZ */
