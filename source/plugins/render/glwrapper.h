#ifndef GLWRAPPER_TLVMZFDN

#define GLWRAPPER_TLVMZFDN

#include "vector"
#include "memory"
#include "unordered_map"
#include "typeinfo"
#include "typeindex"
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
    int getID() const;

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

    std::string getName() const;
    void data(std::shared_ptr<VertexList> l);
    void data(VertexList l);
    void data(std::vector<glm::vec2> l);
    void data(std::vector<glm::vec4> l);
    void setPointer();
    GLint getIndex() const;
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

    std::vector<uint> getSizes() const;
    std::vector<intptr_t> getOffsets() const;

    void data(std::shared_ptr<PolygonList> l);
    void data(const std::vector<uint32_t> &triangles);

private:
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

    GLuint getID() const;

    void bind();
    void release();

    void attachColorTexture(Texture2D *tex);
    void attachDepthTexture(Texture2D *tex);

    void attachColorRenderbuffer(Renderbuffer *rb);
    void attachDepthRenderbuffer(Renderbuffer *rb);

    std::vector<std::string> getAttachments() const;
    int getAttachmentPos(std::string name) const;

private:
    uint color_attachments;
    GLuint id;
    std::vector<std::string> _attachments;
    std::vector<Texture2D*> _textures;
    std::vector<Renderbuffer*> _renderbuffers;
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

    std::string getName() const;
    GLuint getID() const;

    void bind();
    void release();

    Format getFormat() const;

private:
    GLuint _id;
    Format _format;
    uint _width, _height;
    std::string _name;
    bool _initialized;
};

class Texture;
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

    GLuint getID() const;

    int getUniformLocation(std::string name) const;

    static std::string shaderTypeStr(int type);

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

    void setTexture(Texture *texture, std::string name="");

    void bindAttributeLocation(VBO *vbo);
    void bindFragmentLocation(unsigned int index, std::string name);

    bool hasAttribute(std::string name);
    bool hasFragmentOutput(std::string name);

    inline bool isBound() { return _isBound; }

    void bufferedAttribute(std::string name);

    void enableAttribute(std::string name);
    void disableAttribute(std::string name);

    std::string getFileName(int shaderType) const;

    void link();
    void init();

private:
    struct TextureInfo {
        GLuint texture;
        GLenum target;
        std::string name;
    };

    void _addShaderFromSource(std::string src, ShaderType type);

    GLuint _id;
    std::atomic<bool> _isBound, _initialized;
    int _attributes = 0;
    size_t _offset;
    std::mutex _srcLock;
    std::unordered_map<std::string, int> _attributeLocations;
    std::unordered_map<int, std::string> _shaderSources;
    std::vector<TextureInfo> _textures;
    std::unordered_map<int, std::string> _fileNameMap;
};

class UniformState
{
public:
    UniformState(ShaderProgram *prog, std::string name, Property value);
    UniformState(const UniformState &other) = delete;
    UniformState(const UniformState &&other);
    ~UniformState();

    UniformState& operator=(const UniformState &other) = delete;
    UniformState& operator=(const UniformState &&other);

private:
    mutable bool _valid;
    std::string _name;
    Property _oldValue;
    ShaderProgram *_program;
};

class UniformStateManager
{
public:
    UniformStateManager(ShaderProgram *prog);
    ~UniformStateManager();

    void addState(std::string name, Property value);
    void setFromPropertyMap(PropertyMap map);
    void reset();

private:
    ShaderProgram *_program;
    std::vector<UniformState> _states;
};

class Texture
{
public:
    enum Format {
        R,
        R8,
        R16,
        R16F,
        R32F,
        RG,
        RG8,
        RG16,
        RG16F,
        RG32F,
        RGB,
        RGB8,
        RGBA,
        RGBA8,
        RGB16F,
        RGBA16F,
        DEPTH,
        DEPTH16,
        DEPTH32F
    };

    enum Target {
        TEXTURE1D,
        TEXTURE2D,
        TEXTURE_BUFFER
    };

    enum WrapMode {
        REPEAT,
        CLAMP_TO_EDGE
    };

    enum Filter {
        NEAREST,
        LINEAR
    };

    Texture(std::string name, Texture::Format format, Target target=TEXTURE1D);
    virtual ~Texture();

    int width();

    std::string getName() const;
    void setName(std::string name);
    virtual void bind();
    virtual void release();

    virtual void init();

    template<typename T>
    void init(const std::vector<T> &data)
    {
        Texture::init();

        GLenum format = getGLFormat();
        GLenum internalFormat = getGLInternalFormat();
        GLenum type = getGLDataType();

        bind();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage1D(GL_TEXTURE_1D,
                     0,
                     internalFormat,
                     width(),
                     0,
                     format,
                     type,
                     data.data());
        MTGLERROR;
    }

    GLuint getID() const;
    void setFormat(Texture::Format format);
    void setWrapMode(Texture::WrapMode wrap);
    Texture::WrapMode getWrapMode() const;
    Format getFormat() const;
    GLenum getGLFormat() const;
    GLenum getGLDataType() const;
    GLenum getGLInternalFormat() const;
    GLenum getGLWrapMode() const;
    GLenum getGLFilter() const;

    void setFilter(Filter filter);
    Texture::Filter getFilter() const;

    void setWidth(int w);

    GLenum getGLSize() const;

    bool isInitialized() const;
    void generateMipmaps();
    GLenum getGLTarget() const;

protected:
    GLenum getInternalFormat() const;

private:
    GLuint _id;
    Format _format;
    Target _target;
    WrapMode _wrapMode;
    Filter _filter;

    bool _initialized;
    std::string _name;
    int _width;

    bool _genMipmaps;
};

class Texture2D : public Texture
{
public:
    Texture2D(std::string name,
              Texture::Format format = RGBA);
    ~Texture2D();

    int height();
    void setHeight(int h);

    void init();
    void init(std::vector<glm::vec2> data);
    void init(std::vector<unsigned char> data);

protected:

private:
    int _height;
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
