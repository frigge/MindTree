#include "GL/glew.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "iostream"
#include "fstream"
#include "rendertree.h"
#include "data/debuglog.h"

#include "glwrapper.h"

using namespace MindTree::GL;

//#define DEBUG_GL_WRAPPER
//#define DEBUG_GL_WRAPPER1

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
    MTGLERROR;
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &id);
    MTGLERROR;
}

void VAO::bind()    
{
    glBindVertexArray(id);
    MTGLERROR;
    //for(auto &vbo : vbos) vbo->bind();
    bound = true;
}

void VAO::release()    
{
    glBindVertexArray(0);
    MTGLERROR;
    bound = false;
}

Buffer::Buffer(GLenum bufferType)
    : _bufferType(bufferType)
{
#ifdef DEBUG_GL_WRAPPER
    dbout("gen vbo");
#endif
    glGenBuffers(1, &id);
    MTGLERROR;
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &id);
    MTGLERROR;
}

void Buffer::bind()
{
    glBindBuffer(_bufferType, id);
    MTGLERROR;
}

void Buffer::release()    
{
    glBindBuffer(_bufferType, 0);
    MTGLERROR;
}

VBO::VBO(std::string name) : 
    Buffer(GL_ARRAY_BUFFER),
    _name(name),
    _index(RenderTree::getResourceManager()->getIndexForAttribute(name)),
    _size(0), 
    _datatype(GL_FLOAT)
{
#ifdef DEBUG_GL_WRAPPER
    dbout("creating a VBO with name: " << name << " at index: " << _index);
#endif
}

VBO::~VBO()
{
}

void VBO::overrideIndex(uint index)
{
    _index = index;
}

std::string VBO::getName()    
{
    return _name;
}

void VBO::bind()
{
    Buffer::bind();
    glEnableVertexAttribArray(_index);
    MTGLERROR;
}

void VBO::data(std::shared_ptr<VertexList> l)
{
    _datatype = GL_FLOAT;
    _size = 3;

    size_t datasize = l->size() * _size * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, datasize, &(*l)[0], GL_STATIC_DRAW);
}

void VBO::setPointer()
{
    glVertexAttribPointer(_index, _size, _datatype, GL_FALSE, 0, 0);
}

void VBO::data(VertexList l)
{
    _datatype = GL_FLOAT;
    _size = 3;

    glBufferData(GL_ARRAY_BUFFER, l.size() * 3 * sizeof(float), &l[0], GL_STATIC_DRAW);
}

void VBO::data(std::vector<glm::vec2> l)
{
    _datatype = GL_FLOAT;
    _size = 2;

    glBufferData(GL_ARRAY_BUFFER, l.size() * _size * sizeof(float), &l[0], GL_STATIC_DRAW);
}

GLint VBO::getIndex()    
{
    return _index;
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

    _polysizes.clear();
    _indexOffsets.clear();
    
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
    MTGLERROR;

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

void FBO::attachColorTexture(std::weak_ptr<Texture2D> tex)    
{
    for(auto tx : _textures)
        if(tx.lock() == tex.lock())
            return;

    _textures.push_back(tex);
    _attachments.push_back(tex.lock()->getName());

#ifdef DEBUG_GL_WRAPPER
    dbout("attaching texture: " << tex.lock()->getName() << " to color attachment: " << color_attachments);
#endif

    MTGLERROR;
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + color_attachments++,
        GL_TEXTURE_2D,
        tex.lock()->getID(),
        0);
    MTGLERROR;

}

void FBO::attachColorRenderbuffer(std::weak_ptr<Renderbuffer> rb)
{
    for(auto r : _renderbuffers)
        if(r.lock() == rb.lock())
            return;

    _renderbuffers.push_back(rb);
    _attachments.push_back(rb.lock()->getName());

    MTGLERROR;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + color_attachments++,
        GL_RENDERBUFFER,
        rb.lock()->getID());
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

void FBO::attachDepthTexture(std::weak_ptr<Texture2D> tex)    
{
    MTGLERROR;
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        tex.lock()->getID(),
        0);
    MTGLERROR;
}

void FBO::attachDepthRenderbuffer(std::weak_ptr<Renderbuffer> rb)
{
    MTGLERROR;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        rb.lock()->getID());
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
    assert(_initialized);
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

ShaderProgram::ShaderProgram() :
    _id(0),
    _isBound(false),
    _initialized(false),
    _offset(0)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(_id);
    MTGLERROR;
}

void ShaderProgram::init()
{
    assert(RenderThread::id() == std::this_thread::get_id());
    std::lock_guard<std::mutex> lock(_srcLock);
    if(_initialized) return;

    _initialized = true;
    _id = glCreateProgram();
    MTGLERROR;

    for (auto p : _shaderSources)
        _addShaderFromSource(p.second, static_cast<ShaderType>(p.first));

    link();
}

void ShaderProgram::bind()
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);
    if(!_id) return;
    glUseProgram(_id);
    _isBound  = !MTGLERROR;

    for (size_t i = 0; i < _textures.size(); ++i) {
        auto tx = _textures[i].texture;
        if(!tx.expired()) {
            glActiveTexture(GL_TEXTURE0 + i);
            tx.lock()->bind();
        }
    }
}

void ShaderProgram::release()
{
    assert(RenderThread::id() == std::this_thread::get_id());
    if(!_id) return;
    _isBound = false;
    glUseProgram(0);
    MTGLERROR;
}

void ShaderProgram::link()
{
    assert(RenderThread::id() == std::this_thread::get_id());
    _textures.clear();

    glLinkProgram(_id);
    GLint linkStatus;
    glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
        std::cout << "program could not be linked" << std::endl;
    MTGLERROR;
}

void ShaderProgram::addShaderFromSource(std::string src, ShaderProgram::ShaderType type)
{
    std::lock_guard<std::mutex> lock(_srcLock);
    _shaderSources[type] = src;
}

void ShaderProgram::_addShaderFromSource(std::string src, ShaderProgram::ShaderType type)
{
    assert(RenderThread::id() == std::this_thread::get_id());

    std::string shadertype;
    GLenum t = GL_VERTEX_SHADER;
    switch(type)
    {
        case VERTEX:
            t = GL_VERTEX_SHADER;
            shadertype = "Vertex Shader";
            break;
        case FRAGMENT:
            t = GL_FRAGMENT_SHADER;
            shadertype = "Fragment Shader";
            break;
        case GEOMETRY:
            t = GL_GEOMETRY_SHADER;
            shadertype = "Geometry Shader";
            break;
        default:
            break;
    }

    GLuint shader = glCreateShader(t);
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

    std::string filename = _fileNameMap[type];

    if(log != "") {
        std::cout << "=========compile log(" << shadertype << ":" << filename << ")================" << std::endl;
        std::cout << log << std::endl;
        std::cout << "=====================================" << std::endl;
    }

    glAttachShader(_id, shader);
    MTGLERROR;
}

void ShaderProgram::addShaderFromFile(std::string filename, ShaderProgram::ShaderType type)
{
    _fileNameMap[type] = filename;

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

int ShaderProgram::getUniformLocation(std::string name) const
{
    int loc = glGetUniformLocation(_id, name.c_str());
    if(MTGLERROR) dbout(name);
    return loc;
}

void ShaderProgram::setUniform(std::string name, const glm::ivec2 &value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniform2i(location, value.x, value.y);
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, const glm::ivec3 &value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniform3i(location, value.x, value.y, value.z);
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, const glm::vec2 &value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniform2f(location, value.x, value.y);
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, const glm::vec3 &value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniform3f(location, value.x, value.y, value.z);
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, const glm::vec4 &value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniform4f(location, value.x, value.y, value.z, value.w);
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, float value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniform1f(location, value);
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, int value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) {
        glUniform1i(location, value);
    }
    if(MTGLERROR) dbout(name);
}

void ShaderProgram::setUniform(std::string name, const glm::mat4 &value)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    GLint location = getUniformLocation(name);
    if(location > -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    if(MTGLERROR) dbout(name);
}

glm::ivec2 ShaderProgram::getUniformi2(std::string name) const
{
    glm::ivec2 ret;
    glGetUniformiv(_id, getUniformLocation(name), glm::value_ptr(ret));
    if(MTGLERROR) dbout(name);

    return ret;
}

glm::ivec3 ShaderProgram::getUniformi3(std::string name) const
{
    glm::ivec3 ret;
    glGetUniformiv(_id, getUniformLocation(name), glm::value_ptr(ret));
    if(MTGLERROR) dbout(name);

    return ret;
}

glm::vec2 ShaderProgram::getUniformf2(std::string name) const
{
    glm::vec2 ret;
    glGetUniformfv(_id, getUniformLocation(name), glm::value_ptr(ret));
    if(MTGLERROR) dbout(name);

    return ret;
}

glm::vec3 ShaderProgram::getUniformf3(std::string name) const
{
    glm::vec3 ret;
    glGetUniformfv(_id, getUniformLocation(name), glm::value_ptr(ret));
    if(MTGLERROR) dbout(name);

    return ret;
}

glm::vec4 ShaderProgram::getUniformf4(std::string name) const
{
    glm::vec4 ret;
    glGetUniformfv(_id, getUniformLocation(name), glm::value_ptr(ret));
    if(MTGLERROR) dbout(name);

    return ret;
}

float ShaderProgram::getUniformf(std::string name) const
{
    float ret;
    glGetUniformfv(_id, getUniformLocation(name), &ret);
    if(MTGLERROR) dbout(name);

    return ret;
}

int ShaderProgram::getUniformi(std::string name) const
{
    int ret;
    glGetUniformiv(_id, getUniformLocation(name), &ret);
    if(MTGLERROR) dbout(name);

    return ret;
}

glm::mat4 ShaderProgram::getUniformf4x4(std::string name) const
{
    glm::mat4 ret;
    glGetUniformfv(_id, getUniformLocation(name), glm::value_ptr(ret));
    if(MTGLERROR) dbout(name);

    return ret;

}

void ShaderProgram::setUniformFromProperty(std::string name, Property prop)
{
    if(prop.getType() == "FLOAT")
    {
        setUniform(name, static_cast<float>(prop.getData<double>()));
    }

    else if(prop.getType() == "INTEGER")
    {
        setUniform(name, prop.getData<int>());
    }

    else if(prop.getType() == "BOOLEAN")
    {
        setUniform(name, static_cast<int>(prop.getData<bool>()));
    }

    else if(prop.getType() == "COLOR")
    {
        setUniform(name, prop.getData<glm::vec4>());
    }

    else if(prop.getType() == "VECTOR3D")
    {
        setUniform(name, prop.getData<glm::vec3>());
    }

    else if(prop.getType() == "INTVECTOR2D")
    {
        setUniform(name, prop.getData<glm::ivec2>());
    }

    else if(prop.getType() == "MAT4")
    {
        setUniform(name, prop.getData<glm::mat4>());
    }
}

MindTree::Property ShaderProgram::getUniformAsProperty(std::string name, DataType t) const
{
    Property prop;
    if(t == "FLOAT")
        prop = getUniformf(name);

    else if(t == "INTEGER")
        prop = getUniformi(name);

    else if(t == "BOOLEAN")
        prop = getUniformi(name);

    else if(t == "COLOR")
        prop = getUniformf4(name);

    else if(t == "VECTOR3D")
        prop = getUniformf3(name);

    else if(t == "INTVECTOR2D")
        prop = getUniformi2(name);

    else if(t == "MAT4")
        prop = getUniformf4x4(name);

    return prop;
}

void ShaderProgram::setUniforms(PropertyMap map)
{
    for(const auto &p : map) {
        setUniformFromProperty(p.first, p.second);
    }
}

void ShaderProgram::cleanupTextures()
{
    _textures.erase(std::remove_if(begin(_textures), end(_textures), [](TextureInfo tx) {
        return tx.texture.expired();
    }), end(_textures));
}

void ShaderProgram::setTexture(std::shared_ptr<Texture> texture, std::string name)
{
    cleanupTextures();
    assert(RenderThread::id() == std::this_thread::get_id());

    assert(_initialized);

    std::string n;
    if(name == "")
        n = texture->getName();
    else
        n = name;
    int textureSlot;
    auto it = std::find_if(begin(_textures), 
                           end(_textures), 
                           [texture] (TextureInfo other){ 
                               if(other.texture.expired()) return false;
                               return other.texture.lock()->getID() == texture->getID(); 
                           });

    if(it != end(_textures)) {
        textureSlot = std::distance(begin(_textures), it);
    }
    else {
        textureSlot = _textures.size();
        _textures.push_back({texture, n});
    }

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }

    glActiveTexture(GL_TEXTURE0 + textureSlot);
    if(MTGLERROR)
        dbout(textureSlot);

#ifdef DEBUG_GL_WRAPPER1
    dbout("shader " << _id << ": attaching texture: " << n << " to textureSlot: " << textureSlot);
#endif

    texture->bind();
    setUniform(n, textureSlot);

    if(wasntbound) release();
}

void ShaderProgram::bindAttributeLocation(std::shared_ptr<VBO> vbo)
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);
    if(!hasAttribute(vbo->getName())) return;

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }
    glBindAttribLocation(_id, vbo->getIndex(), vbo->getName().c_str());
    MTGLERROR;

    //needs to be relinked so that the binding actually goes into effect
    link();
    if(wasntbound) release();
}

void ShaderProgram::bindFragmentLocation(unsigned int index, std::string name)
{
#ifdef DEBUG_GL_WRAPPER
    dbout("binding fragment location: " << index << " to out variable: " << name);
#endif
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }

    glBindFragDataLocation(_id, index, name.c_str());
    MTGLERROR;

    link();
    if(wasntbound) release();
}

bool ShaderProgram::hasAttribute(std::string name)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }
    GLint location = glGetAttribLocation(_id, name.c_str());
    MTGLERROR;
    if(wasntbound) release();

    return location > -1;
}

bool ShaderProgram::hasFragmentOutput(std::string name)
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }
    GLint location = glGetFragDataLocation(_id, name.c_str());
    MTGLERROR;

    if(wasntbound) release();

    return location > -1;
}

void ShaderProgram::enableAttribute(std::string name)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    glEnableVertexAttribArray(_attributeLocations[name]);
    MTGLERROR;
}

void ShaderProgram::disableAttribute(std::string name)    
{
    assert(RenderThread::id() == std::this_thread::get_id());
    assert(_initialized);

    glDisableVertexAttribArray(_attributeLocations[name]);
    MTGLERROR;
}

std::string ShaderProgram::getFileName(int shaderType)
{
    if(_fileNameMap.find(shaderType) == _fileNameMap.end())
        return "";

    return _fileNameMap[shaderType];
}
UniformState::UniformState(std::weak_ptr<ShaderProgram> prog, std::string name, Property value) : 
    _name(name), 
    _program(prog)
{
    if(prog.expired())
        return;

    _valid = prog.lock()->getUniformLocation(name) > -1;

    if(_valid) {
        _oldValue = prog.lock()->getUniformAsProperty(name, value.getType());
        prog.lock()->setUniformFromProperty(name, value);
    }
}

UniformState::UniformState(const UniformState &&other) :
    _valid(other._valid),
    _name(other._name),
    _oldValue(other._oldValue),
    _program(other._program)
{
    other._valid = false;
}

UniformState& UniformState::operator=(const UniformState&& other)
{
    _name = other._name;
    _program = other._program;
    _oldValue = other._oldValue;
    _valid = other._valid;
    other._valid = false;
    return *this;
}

UniformState::~UniformState()
{
    assert(_program.lock()->isBound());
    if(_valid) _program.lock()->setUniformFromProperty(_name, _oldValue);
}

UniformStateManager::UniformStateManager(std::weak_ptr<ShaderProgram> prog) :
    _program(prog)
{
}

UniformStateManager::~UniformStateManager()
{
}

void UniformStateManager::addState(std::string name, Property value)
{
    _states.emplace_back(_program, name, value);
}

void UniformStateManager::reset()
{
    _states.clear();
}

void UniformStateManager::setFromPropertyMap(PropertyMap map)
{
    for(const auto &p : map) {
        _states.emplace_back(_program, p.first, p.second);
    }
}

Texture::Texture(std::string name, Texture::Format format, Target target)
    : _format(format), 
    _initialized(false), 
    _target(target), 
    _name(name), 
    _id(0),
    _wrapMode(CLAMP_TO_EDGE),
    _width(0),
    _filter(LINEAR),
    _genMipmaps(false)
{
}

Texture::~Texture()
{
#ifdef DEBUG_GL_WRAPPER
    dbout("delete texture: " << _id);
#endif
    glDeleteTextures(1, &_id); 
    MTGLERROR;
}

std::string Texture::getName()
{
    return _name;
}

void Texture::setName(std::string name)
{
    _name = name;
}

void Texture::generateMipmaps()
{
    _genMipmaps = true;
}

namespace {
    bool isDepthTexture(Texture::Format fm)
    {
        return fm >= Texture::DEPTH;
    }
}

void Texture::bind()
{
    assert(_initialized);
    GLenum target = getGLTarget();
    glBindTexture(target, _id);
    GLenum wrap = getGLWrapMode();
    GLenum filter = getGLFilter();
    if(isDepthTexture(getFormat())) {
        glTexParameteri(target, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);

    if(_genMipmaps) {
        glGenerateMipmap(target);
        _genMipmaps = false;
    }
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
        case TEXTURE1D:
            return GL_TEXTURE_1D;
    }
}

bool Texture::isInitialized() const
{
    return _initialized;
}

void Texture::init()
{
    _initialized = true;

    if(!_id) glGenTextures(1, &_id);
#ifdef DEBUG_GL_WRAPPER
    dbout("generated texture: " << _id);
#endif

    MTGLERROR;
}

void Texture::init(std::vector<unsigned char> data)
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

void Texture::init(std::vector<glm::vec2> data)
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

GLuint Texture::getID() const
{
    return _id; 
}

Texture::Format Texture::getFormat() const
{
    return _format;
}

void Texture::setFormat(Texture::Format format)
{
    _format = format;
}

GLenum Texture::getGLFormat() const
{
    switch(getFormat()) {
        case R:
        case R8:
        case R16:
        case R16F:
        case R32F:
            return GL_RED;
        case RG:
        case RG8:
        case RG16:
        case RG16F:
        case RG32F:
            return GL_RG;
        case RGB:
        case RGB8:
        case RGB16F:
            return GL_RGB;
        case RGBA:
        case RGBA8:
        case RGBA16F:
            return GL_RGBA;
        case DEPTH:
        case DEPTH16:
        case DEPTH32F:
            return GL_DEPTH_COMPONENT;
    }
}

GLenum Texture::getGLDataType() const
{
    switch(getFormat()) {
        case R:
        case R8:
        case RGB:
        case RGB8:
        case RGBA:
        case RGBA8:
        case RG:
        case RG8:
        case DEPTH:
        case DEPTH16:
            return GL_UNSIGNED_BYTE;
        case R16:
        case RG16:
            return GL_INT;
        case R16F:
        case R32F:
        case RG16F:
        case RG32F:
        case RGB16F:
        case RGBA16F:
        case DEPTH32F:
            return GL_FLOAT;
    }
}

GLenum Texture::getGLInternalFormat() const
{
    switch(getFormat()) {
        case R:
            return GL_RED;
        case R8:
            return GL_R8;
        case R16:
            return GL_R16;
        case R16F:
            return GL_R16F;
        case R32F:
            return GL_R32F;
        case RG:
            return GL_RG;
        case RG8:
            return GL_RG8;
        case RG16:
            return GL_RG16;
        case RG16F:
            return GL_RG16F;
        case RG32F:
            return GL_RG32F;
        case RGB:
            return GL_RGB;
        case RGB8:
            return GL_RGB8;
        case RGBA:
            return GL_RGBA;
        case RGBA8:
            return GL_RGBA8;
        case RGB16F:
            return GL_RGB16F;
        case RGBA16F:
            return GL_RGBA16F;
        case DEPTH:
            return GL_DEPTH_COMPONENT;
        case DEPTH16:
            return GL_DEPTH_COMPONENT16;
        case DEPTH32F:
            return GL_DEPTH_COMPONENT32F;
    }
}

GLenum Texture::getGLWrapMode() const
{
    switch(_wrapMode) {
        case CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case REPEAT:
            return GL_REPEAT;
    }
}

GLenum Texture::getGLFilter() const
{
    switch(_filter) {
        case NEAREST: return GL_NEAREST;
        case LINEAR: return GL_LINEAR;
    }
}

void Texture::setWrapMode(WrapMode wrap)
{
    _wrapMode = wrap;
}

void Texture::setFilter(Filter filter)
{
    _filter = filter;
}

Texture::Filter Texture::getFilter() const
{
    return _filter;
}

Texture::WrapMode Texture::getWrapMode() const
{
    return _wrapMode;
}

void Texture::init(std::vector<unsigned char> data)
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

void Texture::init(std::vector<glm::vec2> data)
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

void Texture::setWidth(int width)
{
    _width = width;
}

int Texture::width()
{
    return _width;
}

Texture2D::Texture2D(std::string name, Texture::Format format)
    : Texture(name, format, TEXTURE2D), _height(0)
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::setHeight(int h)
{
    _height = h;
}

void Texture2D::init(std::vector<glm::vec2> data)
{
    Texture::init();

    GLenum format = getGLFormat();
    GLenum internalFormat = getGLInternalFormat();
    GLenum type = getGLDataType();

    bind();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 width(),
                 height(),
                 0,
                 format,
                 type,
                 &data[0]);
    MTGLERROR;
}

void Texture2D::init(std::vector<unsigned char> data)
{
    Texture::init();

    GLenum format = getGLFormat();
    GLenum internalFormat = getGLInternalFormat();
    GLenum type = getGLDataType();

    bind();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 width(),
                 height(),
                 0,
                 format,
                 type,
                 data.data());
    MTGLERROR;
}

void Texture2D::init()
{
    Texture::init();

    GLenum format = getGLFormat();
    GLenum internalFormat = getGLInternalFormat();
    GLenum type = getGLDataType();

    bind();

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 width(),
                 height(),
                 0,
                 format,
                 type,
                 nullptr);
    MTGLERROR;
}

int Texture2D::height()
{
    return _height;
}

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

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::cleanUp()
{
    std::lock_guard<std::mutex> lock(_resourceMutex);
    _scheduledResource.clear();
}

int ResourceManager::getIndexForAttribute(std::string name)
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

std::shared_ptr<VBO> ResourceManager::createVBO(ObjectDataPtr data, std::string name)    
{
    auto &vbos = _vboMap[data];
    auto vbo = std::make_shared<VBO>(name);
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

ContextBinder::ContextBinder(QGLContext *context)
    : _context(context)
{
    _context->makeCurrent();
}

ContextBinder::~ContextBinder()
{
    _context->doneCurrent();
}

QtContext::QtContext()
    : QGLContext(format())
{
}

void QtContext::makeCurrent()
{
    QGLContext::makeCurrent();
}

void QtContext::doneCurrent()
{
    QGLContext::doneCurrent();
}

QGLFormat QtContext::format()
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
    return format;
}
