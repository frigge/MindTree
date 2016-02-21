#include "resource_handling.h"

using namespace MindTree;
using namespace MindTree::GL;

AbstractResource::AbstractResource(std::string name)
    : _name(name)
{
}

AbstractResource::~AbstractResource()
{
#ifdef DEBUG_GL_WRAPPER
    dbout("destroying " << _name);
#endif
}
template<>
const std::string Resource<Texture>::s_resource_name("Texture");

template<>
const std::string Resource<Texture2D>::s_resource_name("Texture2D");

template<>
const std::string Resource<ShaderProgram>::s_resource_name("ShaderProgram");

template<>
const std::string Resource<FBO>::s_resource_name("FBO");

template<>
const std::string Resource<VBO>::s_resource_name("VBO");

template<>
const std::string Resource<IBO>::s_resource_name("IBO");

template<>
const std::string Resource<VAO>::s_resource_name("VAO");

template<>
const std::string Resource<Renderbuffer>::s_resource_name("Renderbuffer");

ResourceManager::ResourceManager() :
    shaderManager_(std::make_unique<ShaderManager>(this)),
    geometryCache_(std::make_unique<GeometryCache>(this))
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::cleanUp()
{
    _scheduledResource.clear();
}

GeometryCache::GeometryCache(ResourceManager *manager) :
    manager_(manager)
{
}

int GeometryCache::getIndexForAttribute(std::string name)
{
    uint index = -1;
    if(_attributeIndexMap.find(name) == end(_attributeIndexMap)) {
        index = _attributeIndexMap.size();
        _attributeIndexMap[name] = index;
    }
    else
        index = _attributeIndexMap.at(name);

    return index;
}

VBO* GeometryCache::createVBO(ObjectData *data, std::string name)
{
    auto &vbos = _vboMap[data];
    auto vbo = make_resource<VBO>(manager_, name);
    vbo->overrideIndex(getIndexForAttribute(name));
    auto *ret = vbo.get();
    vbos.push_back(std::move(vbo));
    return ret;
}

VBO* GeometryCache::getVBO(ObjectData *data, std::string name)
{
    auto &vbos = _vboMap[data];
    for(auto &vbo : vbos)
        if(vbo->getName() == name)
            return vbo.get();

    auto vbo = createVBO(data, name);
    return vbo;
}

void GeometryCache::uploadData(ObjectData *data, std::string name)
{
    auto &vbos = _vboMap[data];

    for(auto &vbo : vbos)
        if(vbo->getName() == name) {
            vbo->bind();
            vbo->setPointer();
            return;
        }

    auto vbo = createVBO(data, name);

    vbo->bind();
    vbo->data(data->getProperty(name).getData<std::shared_ptr<VertexList>>());
    vbo->setPointer();
}

IBO* GeometryCache::createIBO(ObjectData *data)
{
    auto ibo = make_resource<IBO>(manager_);
    auto *ptr = ibo.get();
    _iboMap[data] = std::move(ibo);

    return ptr;
}

IBO* GeometryCache::getIBO(ObjectData *data)
{
    if(_iboMap.find(data) != _iboMap.end())
        return _iboMap[data].get();

    return createIBO(data);
}
