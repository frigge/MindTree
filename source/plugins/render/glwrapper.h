#ifndef GLWRAPPER_TLVMZFDN

#define GLWRAPPER_TLVMZFDN

#include "vector"
#include "memory"
#include "GL/glew.h"

#include "QGLFormat"
#include "QGLContext"

#include "../datatypes/Object/object.h"

namespace MindTree
{
namespace GL
{

class FBO;
inline bool getGLError(std::string location)
{
    switch(glGetError()) {
    case GL_NO_ERROR:
        return false;
    case GL_INVALID_ENUM:
        std::cout << "GL_INVALID_ENUM" ;
        break;
    case GL_INVALID_VALUE:
        std::cout << "GL_INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        std::cout << "GL_INVALID_OPERATION";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
    case GL_OUT_OF_MEMORY:
        std::cout << "GL_OUT_OF_MEMORY";
        break;
    case GL_STACK_UNDERFLOW:
        std::cout << "GL_STACK_UNDERFLOW";
        break;
    case GL_STACK_OVERFLOW:
        std::cout << "GL_STACK_OVERFLOW";
        break;
    }

#ifdef ASSERT_ON_MTGLERROR
    assert(false);
#endif

    std::cout << " in " << location << std::endl;
    return true;
}

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
    VBO(std::string name);
    virtual ~VBO();

    void bind();

    std::string getName();
    void data(std::shared_ptr<VertexList> l);
    void data(VertexList l);
    void data(std::vector<glm::vec2> l);
    void setPointer();
    GLint getIndex();
    void overrideIndex(uint index);

private:
    GLuint _index;
    GLenum _datatype;
    uint _size;
    std::string _name;
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
class Renderbuffer;
class FBO
{
public:
    FBO();
    virtual ~FBO();

    GLuint getID();

    void bind();
    void release();

    void attachColorTexture(std::shared_ptr<Texture2D> tex);
    void attachDepthTexture(std::shared_ptr<Texture2D> tex);

    void attachColorRenderbuffer(std::shared_ptr<Renderbuffer> rb);
    void attachDepthRenderbuffer(std::shared_ptr<Renderbuffer> rb);

    std::vector<std::string> getAttachments();
    int getAttachmentPos(std::string name);

private:
    uint color_attachments;
    GLuint id;
    std::vector<std::string> _attachments;
    std::vector<std::shared_ptr<Texture2D>> _textures;
    std::vector<std::shared_ptr<Renderbuffer>> _renderbuffers;
};

class Renderbuffer
{
public:
    enum Format {
        RGB,
        RGB8,
        RGBA,
        RGBA8,
        RGBA16F,
        DEPTH,
        DEPTH16,
        DEPTH32F
    };

    Renderbuffer(std::string name, Renderbuffer::Format format = RGBA, uint width = 1, uint height = 1);
    ~Renderbuffer();

    void init();

    int width();
    int height();
    void setWidth(int w);
    void setHeight(int h);

    std::string getName();
    GLuint getID();

    void bind();
    void release();

    Format getFormat();

private:
    GLuint _id;
    Format _format;
    uint _width, _height;
    std::string _name;
    bool _initialized;
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

    GLuint getID();

    int getUniformLocation(std::string name) const;

    glm::ivec2 getUniformi2(std::string name) const;
    glm::ivec3 getUniformi3(std::string name) const;
    glm::vec2 getUniformf2(std::string name) const;
    glm::vec3 getUniformf3(std::string name) const;
    glm::vec4 getUniformf4(std::string name) const;
    float getUniformf(std::string name) const;
    int getUniformi(std::string name) const;
    glm::mat4 getUniformf4x4(std::string name) const;

    void setUniform(std::string name, const glm::ivec2 &value);
    void setUniform(std::string name, const glm::ivec3 &value);
    void setUniform(std::string name, const glm::vec2 &value);
    void setUniform(std::string name, const glm::vec3 &value);
    void setUniform(std::string name, const glm::vec4 &value);
    void setUniform(std::string name, float value);
    void setUniform(std::string name, int value);
    void setUniform(std::string name, const glm::mat4 &value);
    void setUniformFromProperty(std::string name, Property prop);
    Property getUniformAsProperty(std::string name, DataType t) const;

    void setUniforms(PropertyMap map);

    void setTexture(std::shared_ptr<Texture2D> texture, std::string name="");

    void bindAttributeLocation(std::shared_ptr<VBO> vbo);
    void bindFragmentLocation(unsigned int index, std::string name);

    bool hasAttribute(std::string name);
    bool hasFragmentOutput(std::string name);

    inline bool isBound() { return _isBound; }

    void bufferedAttribute(std::string name);

    void enableAttribute(std::string name);
    void disableAttribute(std::string name);

    void link();
    void init();

private:
    void _addShaderFromSource(std::string src, ShaderType type);

    GLuint _id;
    std::atomic<bool> _isBound, _initialized;
    int _attributes = 0;
    size_t _offset;
    std::mutex _srcLock;
    std::unordered_map<std::string, int> _attributeLocations;
    std::unordered_map<int, std::string> _shaderSources;
    std::vector<std::shared_ptr<Texture2D>> _textures;
    std::unordered_map<int, std::string> _fileNameMap;
};

class UniformState
{
public:
    UniformState(std::shared_ptr<ShaderProgram> prog, std::string name, Property value);
    UniformState(const UniformState &other) = delete;
    UniformState(const UniformState &&other);
    ~UniformState();

    UniformState& operator=(const UniformState &other) = delete;
    UniformState& operator=(const UniformState &&other);

private:
    mutable bool _valid;
    std::string _name;
    Property _oldValue;
    std::shared_ptr<ShaderProgram> _program;
};

class UniformStateManager
{
public:
    UniformStateManager(std::shared_ptr<ShaderProgram> prog);
    ~UniformStateManager();

    void addState(std::string name, Property value);
    void setFromPropertyMap(PropertyMap map);
    void reset();

private:
    std::shared_ptr<ShaderProgram> _program;
    std::vector<UniformState> _states;
};

class Texture
{
public:
    enum Format {
        RGB,
        RGB8,
        RGBA,
        RGBA8,
        RGBA16F,
        DEPTH,
        DEPTH16,
        DEPTH32F
    };

    enum Target {
        TEXTURE2D
    };

    Texture(std::string name, Texture::Format format, Target target);
    std::string getName();
    void setName(std::string name);
    virtual ~Texture();
    virtual void bind();
    virtual void release();
    GLuint getID();
    void setFormat(Texture::Format format);
    Format getFormat();

    GLenum getGLSize();

    virtual void init();

protected:
    void invalidate();

private:
    GLenum getGLTarget();

    GLuint _id;
    Format _format;
    Target _target;

    bool _initialized;
    std::string _name;
};

class Texture2D : public Texture
{
public:
    Texture2D(std::string name, 
              Texture::Format format = RGBA, 
              int width = 1, 
              int height = 1);
    ~Texture2D();

    int width();
    int height();
    void setWidth(int w);
    void setHeight(int h);

    void bind();
    void init();

protected:

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

    void scheduleCleanUp(std::shared_ptr<ShaderProgram> prog);
    void scheduleCleanUp(std::shared_ptr<VBO> vbo);
    void scheduleCleanUp(std::shared_ptr<VAO> vao);
    void scheduleCleanUp(std::shared_ptr<IBO> ibo);
    void scheduleCleanUp(std::shared_ptr<Texture> texture);
    void scheduleCleanUp(std::shared_ptr<Renderbuffer> texture);
    int getIndexForAttribute(std::string name);

private:
    friend class RenderManager;

    std::shared_ptr<VBO> createVBO(ObjectDataPtr data, std::string name);
    std::shared_ptr<IBO> createIBO(ObjectDataPtr data);
    void cleanUp();

    std::vector<std::shared_ptr<ShaderProgram>> _scheduledShaders;
    std::vector<std::shared_ptr<VBO>> _scheduledVBOs;
    std::vector<std::shared_ptr<IBO>> _scheduledIBOs;
    std::vector<std::shared_ptr<VAO>> _scheduledVAOs;
    std::vector<std::shared_ptr<Texture>> _scheduledTextures;
    std::vector<std::shared_ptr<Renderbuffer>> _scheduledRenderbuffers;

    std::unordered_map<ObjectDataPtr, std::vector<std::shared_ptr<VBO>>> _vboMap;
    std::unordered_map<ObjectDataPtr, std::shared_ptr<IBO>> _iboMap;
    std::unordered_map<std::string, int> _attributeIndexMap;
};

class ContextBinder
{
public:
    ContextBinder(QGLContext *context);
    ~ContextBinder();

private:
    QGLContext *_context;
};

class QtContext : public QGLContext
{
public:
    QtContext();

    void makeCurrent() override;
    void doneCurrent() override;
    static QGLFormat format();
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: GLWRAPPER_TLVMZFDN */
