#ifndef GLWRAPPER_TLVMZFDN

#define GLWRAPPER_TLVMZFDN

#include "vector"
#include "memory"
#include "GL/gl.h"
#include "QGLFormat"
#include "../datatypes/Object/object.h"

namespace MindTree
{
namespace GL
{

class FBO;
bool getGLError(std::string location);
bool getGLFramebufferError(std::string location);

#define MTGLERROR getGLError(std::string(__PRETTY_FUNCTION__) + ":" + std::to_string(__LINE__))

template<typename T>
class GLObjectBinder
{
public:
    GLObjectBinder(T glObj) : _glObj(glObj) 
    { 
        if(_glObj)
            _glObj->bind(); 
    }
    ~GLObjectBinder() { if(_glObj) _glObj->release(); }

private:
    T _glObj;

};

class VBO;
class IBO;

class VAO
{
public:
    VAO();
    virtual ~VAO();

    void bind();
    void release();

private:
    GLuint id;
    bool bound;
};

class Buffer
{
public:
    Buffer(GLenum bufferType);
    virtual ~Buffer();

    virtual void bind();
    virtual void release();

private:
    GLenum _bufferType;
    GLuint id;
};

class VBO : public Buffer
{
public:
    VBO(std::string name, GLuint index);
    virtual ~VBO();

    void bind();

    std::string getName();
    void data(std::shared_ptr<VertexList> l);
    void data(VertexList l);
    void data(std::vector<glm::vec2> l);
    GLint getIndex();

private:
    GLuint index;
    std::string name;
};

class IBO : public Buffer
{
public:
    IBO();
    virtual ~IBO();

    std::vector<uint> getSizes();
    std::vector<intptr_t> getOffsets();

    void data(std::shared_ptr<PolygonList> l);

private:
    void generateIndices();

    std::vector<uint> _polysizes;
    std::vector<intptr_t> _indexOffsets;
};

class UBO : public Buffer
{
public:
    UBO();
    virtual ~UBO();
};

class Texture2D;
class FBO
{
public:
    FBO();
    virtual ~FBO();

    GLuint getID();

    void bind();
    void release();

    void attachColorTexture(std::string name, std::shared_ptr<Texture2D> tex);
    void attachDepthTexture(std::shared_ptr<Texture2D> tex);

    std::vector<std::string> getAttachements();

private:
    uint color_attachements;
    GLuint id;
    std::vector<std::string> _attachements;
    std::vector<std::shared_ptr<Texture2D>> _textures;
};

class ShaderProgram
{
public:
    enum ShaderType {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        TESSELATION_EVALUATION,
        TESSELATION_CONTROL,
        COMPUTE
    };

    ShaderProgram();
    ShaderProgram(const ShaderProgram&) = delete;
    virtual ~ShaderProgram();

    void bind();
    void release();

    void addShaderFromSource(std::string src, ShaderType type);
    void addShaderFromFile(std::string filename, ShaderType type);

    void setUniform(std::string name, const glm::vec3 &value);
    void setUniform(std::string name, const glm::vec4 &value);
    void setUniform(std::string name, float value);
    void setUniform(std::string name, int value);
    void setUniform(std::string name, const glm::mat4 &value);

    void setTexture(std::string name, std::shared_ptr<Texture2D> texture);
    void resetTextureSlots();

    void bindAttributeLocation(unsigned int index, std::string name);
    void bindFragmentLocation(unsigned int index, std::string name);

    void vertexAttribute(std::string name, const std::vector<int> &data);
    void vertexAttribute(std::string name, const std::vector<glm::vec3> &data);

    bool hasAttribute(std::string name);
    bool hasFragmentOutput(std::string name);

    void bufferedAttribute(std::string name);

    void enableAttribute(std::string name);
    void disableAttribute(std::string name);

    void link();

private:
    void _addShaderFromSource(std::string src, GLenum type);
    void init();

    GLuint _id;
    bool _isBound, _initialized;
    int _attributes = 0;
    size_t _offset;
    std::unordered_map<std::string, int> _attributeLocations;
    std::string _vertexSource;
    std::string _fragmentSource;
    std::string _geometrySource;
    std::string _tessControlSource;
    std::string _tessEvalSource;
    std::string _computeSource;
    std::vector<std::shared_ptr<Texture2D>> _textures;
};

class Texture
{
public:
    enum Channels {
        RGB,
        RGBA,
        DEPTH
    };

    Texture(Texture::Channels channels);
    virtual ~Texture();
    virtual void bind() = 0;
    virtual void release() = 0;
    GLuint getID();
    void setChannels(Texture::Channels channels);
    Channels getChannels();

protected:
    GLuint _id;
    Channels _channels;
};

class Texture2D : public Texture
{
public:
    Texture2D(Texture::Channels channels, int width, int height);
    ~Texture2D();

    int width();
    int height();

    void bind();
    void release();

    void create(void* data = 0);

private:
    int _width, _height;
};

class ResourceManager
{
public:
    ResourceManager();
    virtual ~ResourceManager();

    std::shared_ptr<VBO> getVBO(ObjectDataPtr data, std::string name);
    std::shared_ptr<IBO> getIBO(ObjectDataPtr data);
    void uploadData(ObjectDataPtr data, std::string name);

    std::shared_ptr<VAO> getVAO(ObjectDataPtr data);

    void scheduleCleanUp(std::shared_ptr<ShaderProgram> prog);
    void scheduleCleanUp(std::shared_ptr<VAO> vao);
    void scheduleCleanUp(std::shared_ptr<VBO> vbo);

private:
    friend class RenderManager;

    std::shared_ptr<VBO> createVBO(ObjectDataPtr data, std::string name);
    std::shared_ptr<IBO> createIBO(ObjectDataPtr data);
    void cleanUp();

    std::vector<std::shared_ptr<ShaderProgram>> _scheduledShaders;
    std::vector<std::shared_ptr<VAO>> _scheduledVAOs;
    std::vector<std::shared_ptr<VBO>> _scheduledVBOs;

    std::unordered_map<ObjectDataPtr, std::vector<std::shared_ptr<VBO>>> _vboMap;
    std::unordered_map<ObjectDataPtr, std::shared_ptr<IBO>> _iboMap;
    std::unordered_map<ObjectDataPtr, std::shared_ptr<VAO>> vao_map;
};

class Context
{
public:
    Context();

    virtual void makeCurrent();
    virtual void doneCurrent();
    virtual void swapBuffers() = 0;

    std::shared_ptr<ResourceManager> getManager();

    static Context* getCurrent();
    static Context* getSharedContext();

protected:
    static Context* _sharedContext;
    static Context* currentContext;

private:
    std::shared_ptr<ResourceManager> manager;
};

class SharedContextRAII
{
public:
    SharedContextRAII();
    ~SharedContextRAII();
};

class QtContext : public Context
{
public:
    QtContext();

    static QtContext* getContext();

    void makeCurrent();
    void doneCurrent();
    void swapBuffers();

    QGLContext* getNativeContext();
    static QtContext* getSharedContext();

private:
    QGLContext _context;
    QGLFormat format;
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: GLWRAPPER_TLVMZFDN */
