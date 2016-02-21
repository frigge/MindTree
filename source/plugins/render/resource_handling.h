#ifndef MT_GL_RESOURCE_HANDLING_H
#define MT_GL_RESOURCE_HANDLING_H

#include "glwrapper.h"

namespace MindTree {
namespace GL {

class AbstractResource
{
public:
    AbstractResource(std::string name);
    virtual ~AbstractResource();

private:
    std::string _name;
};

template<class T>
class Resource : public AbstractResource
{
public:
    explicit Resource(std::unique_ptr<T> &&resource) :
        AbstractResource(s_resource_name),
            _resource{std::forward<std::unique_ptr<T>>(resource)} {};

private:
    std::unique_ptr<T> _resource;

    static const std::string s_resource_name;
};

class ShaderManager;
class GeometryCache;

class ResourceManager
{
public:
    ResourceManager();
    virtual ~ResourceManager();

    GeometryCache* geometryCache() { return geometryCache_.get(); }

    template<class T>
    void scheduleCleanUp(T* resource)
    {
        auto res = std::make_unique<Resource<T>>(std::unique_ptr<T>(resource));
        _scheduledResource.push_back(std::move(res));
    }

    inline ShaderManager* shaderManager() {return shaderManager_.get();}

private:
    friend class RenderTree;

    VBO* createVBO(ObjectData *data, std::string name);
    IBO* createIBO(ObjectData *data);
    void cleanUp();

    std::unique_ptr<ShaderManager> shaderManager_;
    std::vector<std::unique_ptr<AbstractResource>> _scheduledResource;
    std::unique_ptr<GeometryCache> geometryCache_;
};

template<typename T>
struct ResourceDeleter {
    ResourceDeleter() : manager_(nullptr) {}
    ResourceDeleter(ResourceManager *manager) : manager_(manager) {}

    void operator()(T *p)
    {
        manager_->scheduleCleanUp(p);
    }

    ResourceManager *manager_;
};

//just as a compile-time guard to not accidentally create std::shared_ptr by hand
// without custom deleter
template<typename T>
using ResourceHandle = std::unique_ptr<T, ResourceDeleter<T>>;

template<typename T, typename ...Args>
ResourceHandle<T> make_resource(ResourceManager *manager, Args ...args)
{
    auto *resource = new T(args...);
    return ResourceHandle<T>(resource, ResourceDeleter<T>(manager));
}

class GeometryCache
{
public:
    GeometryCache(ResourceManager *manager);

    VBO* createVBO(ObjectData *data, std::string name);
    IBO* createIBO(ObjectData *data);
    VBO* getVBO(ObjectData *data, std::string name);
    IBO* getIBO(ObjectData *data);

    void clean(ObjectData*);

    void uploadData(ObjectData *data, std::string name);
    int getIndexForAttribute(std::string name);

private:
    std::unordered_map<ObjectData*, std::vector<ResourceHandle<VBO>>> _vboMap;
    std::unordered_map<ObjectData*, ResourceHandle<IBO>> _iboMap;
    std::unordered_map<std::string, int> _attributeIndexMap;
    ResourceManager *manager_;
};

template<typename T>
struct ShaderFiles {
    static const std::string vertexShader;
    static const std::string geometryShader;
    static const std::string fragmentShader;
    static const std::string tessControlShader;
    static const std::string tessEvalShader;
};

template<typename T>
const std::string ShaderFiles<T>::vertexShader;
template<typename T>
const std::string ShaderFiles<T>::geometryShader;
template<typename T>
const std::string ShaderFiles<T>::fragmentShader;
template<typename T>
const std::string ShaderFiles<T>::tessControlShader;
template<typename T>
const std::string ShaderFiles<T>::tessEvalShader;

template<typename T>
struct ShaderProvider {
    static ResourceHandle<ShaderProgram> provideProgram(ResourceManager *manager) {
        auto prog = make_resource<ShaderProgram>(manager);
        auto vert = ShaderFiles<T>::vertexShader;
        auto frag = ShaderFiles<T>::fragmentShader;
        auto geo = ShaderFiles<T>::geometryShader;
        auto tessc = ShaderFiles<T>::tessControlShader;
        auto tesse = ShaderFiles<T>::tessEvalShader;

        if(vert != "")prog->addShaderFromFile(vert, ShaderProgram::VERTEX);
        if(frag != "")prog->addShaderFromFile(frag, ShaderProgram::FRAGMENT);
        if(geo != "")prog->addShaderFromFile(vert, ShaderProgram::GEOMETRY);
        if(tessc != "")prog->addShaderFromFile(frag, ShaderProgram::TESSELATION_CONTROL);
        if(tesse != "")prog->addShaderFromFile(frag, ShaderProgram::TESSELATION_EVALUATION);
        return prog;
    }
};

class ShaderManager
{
public:
    ShaderManager(ResourceManager *manager) : manager_(manager) {}

    template<typename T>
    ShaderProgram *getProgram()
    {
        auto t = std::type_index(typeid(T));
        if(programs_.find(t) == programs_.end()) {
            programs_[t] = ShaderProvider<T>::provideProgram(manager_);
        }
        return programs_[t].get();
    }

private:
    std::unordered_map<std::type_index, ResourceHandle<ShaderProgram>> programs_;
    ResourceManager *manager_;
};

}
}
#endif
