#include "GL/glew.h"
#include "QGLContext"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "fstream"
#include "glwrapper.h"

using namespace MindTree::GL;

bool MindTree::GL::getGLError(std::string location)
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

    std::cout << " in " << location << std::endl;
    return true;
}

bool MindTree::GL::getGLFramebufferError(std::string location)
{
    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    case GL_FRAMEBUFFER_COMPLETE:
        return false;
    case GL_FRAMEBUFFER_UNDEFINED:
        std::cout << "GL_FRAMEBUFFER_UNDEFINED";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;
    default:
        std::cout << "some other misterious framebuffer error occured";
    }
    std::cout << "in location: " << location << std::endl;
    return true;
}

VAO::VAO()
    : bound(false)
{
    glGenVertexArrays(1, &id);
    getGLError(__PRETTY_FUNCTION__);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &id);
    getGLError(__PRETTY_FUNCTION__);
}

void VAO::bind()    
{
    glBindVertexArray(id);
    getGLError(__PRETTY_FUNCTION__);
    //for(auto &vbo : vbos) vbo->bind();
    bound = true;
}

void VAO::release()    
{
    glBindVertexArray(0);
    getGLError(__PRETTY_FUNCTION__);
    bound = false;
}

Buffer::Buffer(GLenum bufferType)
    : _bufferType(bufferType)
{
    glGenBuffers(1, &id);
    getGLError(__PRETTY_FUNCTION__);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &id);
    getGLError(__PRETTY_FUNCTION__);
}

void Buffer::bind()
{
    glBindBuffer(_bufferType, id);
    getGLError(__PRETTY_FUNCTION__);
}

void Buffer::release()    
{
    glBindBuffer(_bufferType, 0);
    getGLError(__PRETTY_FUNCTION__);
}

VBO::VBO(std::string name, GLuint index)
    : Buffer(GL_ARRAY_BUFFER),
        name(name), index(index)
{
}

VBO::~VBO()
{
}

std::string VBO::getName()    
{
    return name;
}

void VBO::bind()
{
    Buffer::bind();
    getGLError(__PRETTY_FUNCTION__);
}

void VBO::data(std::shared_ptr<VertexList> l)
{
    glBufferData(GL_ARRAY_BUFFER, l->size() * 3 * sizeof(float), &(*l)[0], GL_STATIC_DRAW);
    getGLError(__PRETTY_FUNCTION__);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
    getGLError(__PRETTY_FUNCTION__);
}

void VBO::data(VertexList l)
{
    glBufferData(GL_ARRAY_BUFFER, l.size() * 3 * sizeof(float), &l[0], GL_STATIC_DRAW);
    getGLError(__PRETTY_FUNCTION__);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
    getGLError(__PRETTY_FUNCTION__);
}

void VBO::data(std::vector<glm::vec2> l)
{
    glBufferData(GL_ARRAY_BUFFER, l.size() * 2 * sizeof(float), &l[0], GL_STATIC_DRAW);
    getGLError(__PRETTY_FUNCTION__);
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 0, 0);
    getGLError(__PRETTY_FUNCTION__);
}

GLint VBO::getIndex()    
{
    return index;
}

IBO::IBO()
    : Buffer(GL_ELEMENT_ARRAY_BUFFER)
{
}

IBO::~IBO()
{
}


void IBO::data(std::shared_ptr<PolygonList> l)
{
    int size = 0;
    
    //find out size of index buffer by adding together polygon sizes
    
    std::vector<uint> data;

    intptr_t offset = 0;
    for(const auto &p : *l) {
        size += p.size();
        for(auto v : p.verts())
            data.push_back(v);

        //get the size of each polygon
        _indexOffsets.push_back(offset);
        _polysizes.push_back(p.size());
        offset += p.size() * sizeof(uint);
    }
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint), &data[0], GL_STATIC_DRAW);
    getGLError(__PRETTY_FUNCTION__);

    //cache indices for glMultiDrawElements
}

std::vector<intptr_t> IBO::getOffsets()
{
    return _indexOffsets;
}

std::vector<uint> IBO::getSizes()
{
    return _polysizes;
}

UBO::UBO()
    : Buffer(GL_UNIFORM_BUFFER)
{
}

UBO::~UBO()
{
}

FBO::FBO()
    : color_attachments(0)
{
    glGenFramebuffers(1, &id);
    MTGLERROR;
}

FBO::~FBO()
{
    glDeleteFramebuffers(1, &id);
    MTGLERROR;
}

GLuint FBO::getID()
{
    return id;
}

void FBO::bind()    
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    MTGLERROR;
}

void FBO::release()    
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    MTGLERROR;
}

void FBO::attachColorTexture(std::shared_ptr<Texture2D> tex)    
{
    if(std::find(begin(_textures), end(_textures), tex) != end(_textures))
        return;

    _textures.push_back(tex);
    _attachments.push_back(tex->getName());

    MTGLERROR;
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + color_attachments++,
        GL_TEXTURE_2D,
        tex->getID(),
        0);
    MTGLERROR;

}

void FBO::attachColorRenderbuffer(std::shared_ptr<Renderbuffer> rb)
{
    if(std::find(begin(_renderbuffers), end(_renderbuffers), rb) != end(_renderbuffers))
        return;

    _renderbuffers.push_back(rb);
    _attachments.push_back(rb->getName());

    MTGLERROR;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + color_attachments++,
        GL_RENDERBUFFER,
        rb->getID());
    MTGLERROR;

}

int FBO::getAttachmentPos(std::string name)
{
    int index = 0;
    for(auto attachment : _attachments) {
        if(attachment == name)
            break;
        ++index;
    }

    std::cout << "attachment: " << name << " is at pos: " << index << std::endl;
    return index;
}

void FBO::attachDepthTexture(std::shared_ptr<Texture2D> tex)    
{
    MTGLERROR;
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        tex->getID(),
        0);
    MTGLERROR;
}

void FBO::attachDepthRenderbuffer(std::shared_ptr<Renderbuffer> rb)
{
    MTGLERROR;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        rb->getID());
    MTGLERROR;
}

std::vector<std::string> FBO::getAttachments()
{
    return _attachments;
}

Renderbuffer::Renderbuffer(std::string name, Renderbuffer::Format format, uint width, uint height)
    : _id(0), _width(width), _height(height), _format(format), _name(name), _initialized(false)
{
}

Renderbuffer::~Renderbuffer()
{
    glDeleteRenderbuffers(1, &_id);
}

int Renderbuffer::width()
{
    return _width;
}

int Renderbuffer::height()
{
    return _height;
}

void Renderbuffer::setWidth(int w)
{
    _width = w;
    _initialized = false;
}

void Renderbuffer::setHeight(int h)
{
    _height = h;
    _initialized = false;
}

void Renderbuffer::init()
{
    _initialized = true;

    if(!_id) glGenRenderbuffers(1, &_id);

    glBindRenderbuffer(GL_RENDERBUFFER, _id);
    MTGLERROR;
    GLenum _internalFormat;
    switch(getFormat()) {
        case RGB:
            _internalFormat = GL_RGB;
            break;
        case RGB8:
            _internalFormat = GL_RGB8;
            break;
        case RGBA:
            _internalFormat = GL_RGBA;
            break;
        case RGBA8:
            _internalFormat = GL_RGBA8;
            break;
        case RGBA16F:
            _internalFormat = GL_RGBA16F;
            break;
        case DEPTH:
            _internalFormat = GL_DEPTH_COMPONENT;
            break;
        case DEPTH16:
            _internalFormat = GL_DEPTH_COMPONENT16;
            break;
        case DEPTH32F:
            _internalFormat = GL_DEPTH_COMPONENT32F;
            break;
    }

    glRenderbufferStorage(GL_RENDERBUFFER,
                 _internalFormat,
                 _width,
                 _height);
    MTGLERROR;
}

std::string Renderbuffer::getName()
{
    return _name;
}

GLuint Renderbuffer::getID()
{
    return _id;
}

void Renderbuffer::bind()
{
    if(!_initialized) 
        init();
    else
        glBindRenderbuffer(GL_RENDERBUFFER, _id);
}

void Renderbuffer::release()
{
    glBindRenderbuffer(GL_RENDERBUFFER, _id);
}

Renderbuffer::Format Renderbuffer::getFormat()
{
    return _format;
}

ShaderProgram::ShaderProgram()
    : _isBound(false), _initialized(false), _id(0), _offset(0)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(_id);
    MTGLERROR;
}

void ShaderProgram::init()
{
    _initialized = true;
    if(_id) glDeleteProgram(_id);

    _id = glCreateProgram();
    MTGLERROR;

    _addShaderFromSource(_vertexSource, GL_VERTEX_SHADER);
    _addShaderFromSource(_fragmentSource, GL_FRAGMENT_SHADER);

    link();
}

void ShaderProgram::bind()
{
    if(!_initialized) init();
    if(!_id) return;
    glUseProgram(_id);
    _isBound  = !MTGLERROR;
}

void ShaderProgram::release()    
{
    if(!_id) return;
    _isBound = false;
    glUseProgram(0);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::link()    
{
    _textures.clear();

    glLinkProgram(_id);
    GLint linkStatus;
    glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
        std::cout << "program could not be linked" << std::endl;
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::addShaderFromSource(std::string src, ShaderProgram::ShaderType type)    
{
    switch(type) {
        case VERTEX:
            _vertexSource = src;
            break;
        case FRAGMENT:
            _fragmentSource = src;
            break;
        case GEOMETRY:
            _geometrySource = src;
        case TESSELATION_EVALUATION:
            _tessEvalSource = src;
            break;
        case TESSELATION_CONTROL:
            _tessControlSource = src;
            break;
        case COMPUTE:
            _computeSource = src;
            break;
    }
}

void ShaderProgram::_addShaderFromSource(std::string src, GLenum type)    
{

    GLuint shader = glCreateShader(type);
    MTGLERROR;
    const char* c_str = src.c_str();
    glShaderSource(shader, 1, &c_str, 0);
    MTGLERROR;

    glCompileShader(shader);
    MTGLERROR;
    GLsizei infologlength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, (GLint*)&infologlength);
    GLchar *infolog = new GLchar[infologlength];
    glGetShaderInfoLog(shader, infologlength, 0, infolog);
    std::string log((char*)infolog);
    delete [] infolog;
    std::string shadertype;
    switch(type)
    {
        case GL_VERTEX_SHADER:
            shadertype = "Vertex Shader";
            break;
        case GL_FRAGMENT_SHADER:
            shadertype = "Fragment Shader";
            break;
        case GL_GEOMETRY_SHADER:
            shadertype = "Geometry Shader";
            break;
        default:
            break;
    }
    if(log != "") {
        std::cout << "=========compile log(" << shadertype << "):================" << std::endl;
        std::cout << log << std::endl;
        std::cout << "=====================================" << std::endl;
    }

    glAttachShader(_id, shader);
    MTGLERROR;
}

void ShaderProgram::addShaderFromFile(std::string filename, ShaderProgram::ShaderType type)
{
    std::ifstream stream(filename);

    if(!stream.is_open()) {
        std::cout << "could not open file " << filename << std::endl;
        return;
    }

    std::string content;
    std::string line;
    while (stream) {
        std::getline(stream, line);
        content += line;
        content += "\n";
    }

    addShaderFromSource(content, type);
}

GLuint ShaderProgram::getID()
{
    return _id;
}

void ShaderProgram::setUniform(std::string name, const glm::ivec2 &value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    MTGLERROR;
    if(location > -1) glUniform2i(location, value.x, value.y);
    MTGLERROR;
}

void ShaderProgram::setUniform(std::string name, const glm::ivec3 &value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    MTGLERROR;
    if(location > -1) glUniform3i(location, value.x, value.y, value.z);
    MTGLERROR;
}

void ShaderProgram::setUniform(std::string name, const glm::vec2 &value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(location > -1) glUniform2f(location, value.x, value.y);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::setUniform(std::string name, const glm::vec3 &value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(location > -1) glUniform3f(location, value.x, value.y, value.z);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::setUniform(std::string name, const glm::vec4 &value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(location > -1) glUniform4f(location, value.x, value.y, value.z, value.w);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::setUniform(std::string name, float value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(location > -1) glUniform1f(location, value);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::setUniform(std::string name, int value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(location > -1) {
        glUniform1i(location, value);
    }
    if(getGLError(__PRETTY_FUNCTION__))
       std::cout << "unifrom name: " << name << std::endl;
}

void ShaderProgram::setUniform(std::string name, const glm::mat4 &value)    
{
    if(!_initialized) init();

    GLint location = glGetUniformLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(location > -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    if(getGLError(__PRETTY_FUNCTION__))
       std::cout << "unifrom name: " << name << std::endl;
}

void ShaderProgram::setTexture(std::shared_ptr<Texture2D> texture)
{
    if(!_initialized) init();

    if(std::find(begin(_textures), end(_textures), texture) != end(_textures))
        return;

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }

    int textureSlot = _textures.size();
    _textures.push_back(texture);

    glActiveTexture(GL_TEXTURE0 + textureSlot);

    getGLError(__PRETTY_FUNCTION__);

    texture->bind();
    setUniform(texture->getName(), textureSlot);

    if(wasntbound) release();
}

void ShaderProgram::bindAttributeLocation(unsigned int index, std::string name)    
{
    if(!_initialized) init();
    if(!hasAttribute(name)) return;

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }
    glBindAttribLocation(_id, index, name.c_str()); 
    getGLError(__PRETTY_FUNCTION__);

    //needs to be relinked so that the binding actually goes into effect
    link();
    if(wasntbound) release();
}

void ShaderProgram::bindFragmentLocation(unsigned int index, std::string name)
{
    if(!_initialized) init();

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }

    glBindFragDataLocation(_id, index, name.c_str());
    getGLError(__PRETTY_FUNCTION__);

    link();
    if(wasntbound) release();
}

void ShaderProgram::vertexAttribute(std::string name, const std::vector<glm::vec3> &data)    
{
    if(!_initialized) init();

    if(!_attributeLocations.count(name))
        _attributeLocations[name] = _attributes++;
    uint index = _attributeLocations[name];
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, _offset, &data[0][0]);
    getGLError(__PRETTY_FUNCTION__);
    bindAttributeLocation(index, name);
    _offset += sizeof(glm::vec3) * data.size();
}

void ShaderProgram::vertexAttribute(std::string name, const std::vector<int> &data)    
{
    if(!_initialized) init();

    if(!_attributeLocations.count(name))
        _attributeLocations[name] = _attributes++;
    uint index = _attributeLocations[name];
    glVertexAttribPointer(_id, 1, GL_INT, GL_FALSE, _offset, &data[0]);
    getGLError(__PRETTY_FUNCTION__);
    bindAttributeLocation(_attributeLocations[name], name);
    _offset += sizeof(int) * data.size();
}

bool ShaderProgram::hasAttribute(std::string name)    
{
    if(!_initialized) init();

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }
    GLint location = glGetAttribLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);
    if(wasntbound) release();

    return location > -1;
}

bool ShaderProgram::hasFragmentOutput(std::string name)
{
    if(!_initialized) init();

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }
    GLint location = glGetFragDataLocation(_id, name.c_str());
    getGLError(__PRETTY_FUNCTION__);

    if(wasntbound) release();

    return location > -1;
}

void ShaderProgram::enableAttribute(std::string name)    
{
    if(!_initialized) init();

    glEnableVertexAttribArray(_attributeLocations[name]);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::disableAttribute(std::string name)    
{
    if(!_initialized) init();

    glDisableVertexAttribArray(_attributeLocations[name]);
    getGLError(__PRETTY_FUNCTION__);
}

Texture::Texture(std::string name, Texture::Format format, Target target)
    : _format(format), 
    _initialized(false), 
    _target(target), 
    _name(name), 
    _id(0)
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &_id); 
    getGLError(__PRETTY_FUNCTION__);
}

std::string Texture::getName()
{
    return _name;
}

void Texture::bind()
{
    if(!_initialized) _init();
    glBindTexture(getGLTarget(), _id);
}

void Texture::release()
{
    glBindTexture(getGLTarget(), 0);
}

GLenum Texture::getGLTarget()
{
    switch(_target) {
        case TEXTURE2D:
            return GL_TEXTURE_2D;
    }
}

void Texture::_init()
{
    _initialized = true;

    if(!_id) glGenTextures(1, &_id);

    MTGLERROR;

    init();
}

GLuint Texture::getID()    
{
    return _id; 
}

Texture::Format Texture::getFormat()
{
    return _format;
}

void Texture::setFormat(Texture::Format format)
{
    _format = format;
}

void Texture::invalidate()
{
    _initialized = false;
}

Texture2D::Texture2D(std::string name, Texture::Format format, int width, int height)
    : Texture(name, format, TEXTURE2D), _width(width), _height(height)
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::setWidth(int w)
{
    _width = w;
    invalidate();
}

void Texture2D::setHeight(int h)
{
    _height = h;
    invalidate();
}

void Texture2D::init()
{
    GLenum format, internalFormat, type;
    switch(getFormat()) {
        case RGB:
            format = GL_RGB;
            internalFormat = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            break;
        case RGB8:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            type = GL_UNSIGNED_BYTE;
            break;
        case RGBA:
            format = GL_RGBA;
            internalFormat = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case RGBA8:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            type = GL_UNSIGNED_BYTE;
            break;
        case RGBA16F:
            format = GL_RGBA;
            internalFormat = GL_RGBA16F;
            type = GL_FLOAT;
            break;
        case DEPTH:
            format = GL_DEPTH_COMPONENT;
            internalFormat = GL_DEPTH_COMPONENT;
            type = GL_UNSIGNED_BYTE;
            break;
        case DEPTH16:
            format = GL_DEPTH_COMPONENT;
            internalFormat = GL_DEPTH_COMPONENT16;
            type = GL_UNSIGNED_BYTE;
            break;
        case DEPTH32F:
            format = GL_DEPTH_COMPONENT;
            internalFormat = GL_DEPTH_COMPONENT32F;
            type = GL_FLOAT;
            break;
    }

    glBindTexture(GL_TEXTURE_2D, getID());
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 format,
                 _width,
                 _height,
                 0,
                 internalFormat,
                 type,
                 nullptr);
    MTGLERROR;
}

int Texture2D::width()
{
    return _width;
}

int Texture2D::height()
{
    return _height;
}

void Texture2D::bind()    
{
    Texture::bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if(getFormat() == DEPTH || getFormat() == DEPTH16 || getFormat() == DEPTH32F) {
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
}

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::scheduleCleanUp(std::shared_ptr<ShaderProgram> prog)
{
    _scheduledShaders.push_back(std::move(prog));
}

void ResourceManager::scheduleCleanUp(std::shared_ptr<VAO> vao)
{
    _scheduledVAOs.push_back(std::move(vao));
}

void ResourceManager::scheduleCleanUp(std::shared_ptr<VBO> vbo)
{
    _scheduledVBOs.push_back(vbo);
}

void ResourceManager::cleanUp()
{
    _scheduledShaders.clear();
    _scheduledVAOs.clear();
    _scheduledVBOs.clear();
}

std::shared_ptr<VBO> ResourceManager::createVBO(ObjectDataPtr data, std::string name)    
{
    auto &vbos = _vboMap[data];
    uint index = vbos.size();
    auto vbo = std::make_shared<VBO>(name, index);
    vbos.push_back(vbo);
    return vbo;
}

std::shared_ptr<VBO> ResourceManager::getVBO(ObjectDataPtr data, std::string name)    
{
    auto &vbos = _vboMap[data];
    for(auto vbo : vbos) 
        if(vbo->getName() == name)
            return vbo; 

    auto vbo = createVBO(data, name);
    return vbo;
}

void ResourceManager::uploadData(ObjectDataPtr data, std::string name)
{
    auto &vbos = _vboMap[data];

    for(auto vbo : vbos) 
        if(vbo->getName() == name)
            return;

    auto vbo = createVBO(data, name);
    
    vbo->bind();
    vbo->data(data->getProperty(name).getData<std::shared_ptr<VertexList>>());
}

std::shared_ptr<IBO> ResourceManager::createIBO(ObjectDataPtr data)
{
    auto ibo = std::make_shared<IBO>();    
    _iboMap.insert({data, ibo});

    return ibo;
}

std::shared_ptr<IBO> ResourceManager::getIBO(ObjectDataPtr data)
{
    if(_iboMap.find(data) != _iboMap.end())
        return _iboMap[data];

    return createIBO(data);
}

std::shared_ptr<VAO> ResourceManager::getVAO(ObjectDataPtr data)
{
    if(vao_map.find(data) == vao_map.end()) {
        auto vao = std::make_shared<VAO>();
        vao_map.insert(std::make_pair(data, vao));
    }
    auto &vao = vao_map[data];
    return vao;
}

Context* Context::currentContext = nullptr;
Context* Context::_sharedContext = nullptr;

Context::Context()
    : manager(std::make_shared<ResourceManager>())
{
}

std::shared_ptr<ResourceManager> Context::getManager()    
{
    return manager;
}

Context* Context::getCurrent()    
{
    return currentContext;
}

Context* Context::getSharedContext()
{
    return _sharedContext;
}

void Context::makeCurrent()    
{
    currentContext = this;
}

void Context::doneCurrent()    
{
    currentContext = nullptr;
}

SharedContextRAII::SharedContextRAII()
{
    Context::getSharedContext()->makeCurrent();
}

SharedContextRAII::~SharedContextRAII()
{
    Context::getSharedContext()->doneCurrent();
}

QtContext::QtContext()
    : Context(), _context(QGLFormat::defaultFormat())
{
    auto options = QGL::DoubleBuffer
        | QGL::DepthBuffer
        | QGL::Rgba
        | QGL::DirectRendering
        | QGL::AlphaChannel
        | QGL::StencilBuffer;

    auto format = QGLFormat(options);

    format.setVersion(4, 3);
    format.setProfile(QGLFormat::CompatibilityProfile);

    _context.setFormat(format);
}

void QtContext::makeCurrent()    
{
    Context::makeCurrent();
    if (!_context.isValid()) {
        if (!_context.create()) {
            std::cout << "couldn't create context" << std::endl;
            return;
        }
    }
    _context.makeCurrent();
}

void QtContext::doneCurrent()    
{
    Context::doneCurrent();
    _context.doneCurrent();
}

void QtContext::swapBuffers()
{
    _context.swapBuffers();
}

QGLContext* QtContext::getNativeContext()
{
    return &_context;
}

QtContext* QtContext::getSharedContext()
{
    QtContext* context = nullptr;
    if(!_sharedContext) {
        context = new QtContext();
        _sharedContext = context;
    }
    else {
        context = static_cast<QtContext*>(_sharedContext);
    }

    return context;
}
