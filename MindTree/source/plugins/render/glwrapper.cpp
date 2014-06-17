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
    : color_attachements(0)
{
    glGenFramebuffers(1, &id);
    getGLError(__PRETTY_FUNCTION__);
}

FBO::~FBO()
{
    glDeleteFramebuffers(1, &id);
    getGLError(__PRETTY_FUNCTION__);
}

GLuint FBO::getID()
{
    return id;
}

void FBO::bind()    
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    getGLError(__PRETTY_FUNCTION__);
}

void FBO::release()    
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    getGLError(__PRETTY_FUNCTION__);
}

void FBO::attachColorTexture(std::string name, std::shared_ptr<Texture2D> tex)    
{
    if(std::find(begin(_textures), end(_textures), tex) != end(_textures))
        return;

    _textures.push_back(tex);
    _attachements.push_back(name);

    getGLError("dummy");
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + color_attachements++,
        GL_TEXTURE_2D,
        tex->getID(),
        0);
    getGLError(__PRETTY_FUNCTION__);

    std::cout << "attaching texture: " 
        << name << "(" << tex->getID() << ") to framebuffer attachement: " 
        << color_attachements - 1 << std::endl;
}

void FBO::attachDepthTexture(std::shared_ptr<Texture2D> tex)    
{
    getGLError("dummy");
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        tex->getID(),
        0);
    getGLError(__PRETTY_FUNCTION__);
}

std::vector<std::string> FBO::getAttachements()
{
    return _attachements;
}

ShaderProgram::ShaderProgram()
    : _isBound(false), _initialized(false), _id(0), _offset(0)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(_id);
    getGLError(__PRETTY_FUNCTION__);
}

void ShaderProgram::init()
{
    _initialized = true;
    _id = glCreateProgram();
    getGLError(__PRETTY_FUNCTION__);

    _addShaderFromSource(_vertexSource, GL_VERTEX_SHADER);
    _addShaderFromSource(_fragmentSource, GL_FRAGMENT_SHADER);

    link();
}

void ShaderProgram::bind()
{
    if(!_initialized) init();
    if(!_id) return;
    getGLError("dummy");
    glUseProgram(_id);
    _isBound  = !getGLError(__PRETTY_FUNCTION__);
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
    getGLError(__PRETTY_FUNCTION__);
    const char* c_str = src.c_str();
    glShaderSource(shader, 1, &c_str, 0);
    getGLError(__PRETTY_FUNCTION__);

    glCompileShader(shader);
    getGLError(__PRETTY_FUNCTION__);
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
    getGLError(__PRETTY_FUNCTION__);
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
    if(location > -1) glUniform1i(location, value);
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

void ShaderProgram::setTexture(std::string name, std::shared_ptr<Texture2D> texture)
{
    if(!_initialized) init();

    if(std::find(begin(_textures), end(_textures), texture) != end(_textures))
        return;

    bool wasntbound = false;
    if(!_isBound) {
        wasntbound = true;
        bind();
    }

    glActiveTexture(GL_TEXTURE0 + _textures.size());

    getGLError(__PRETTY_FUNCTION__);
    std::cout << "setting active texture to slot: " << _textures.size() 
        << " on program: " << _id << std::endl;

    texture->bind();
    setUniform(name, (int)_textures.size());
    _textures.push_back(texture);

    if(wasntbound) release();
}

void ShaderProgram::resetTextureSlots()
{
    std::cout << "texture slots of program: " << _id << " are cleared" << std::endl;
    _textures.clear();
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

    std::cout << "binding fragment output: " << name << " to location: " << index << std::endl;

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

Texture::Texture(Texture::Channels channels)
    : _channels(channels)
{
    glGenTextures(1, &_id);
    getGLError(__PRETTY_FUNCTION__);
}

Texture::~Texture()
{
    glDeleteTextures(1, &_id); 
    getGLError(__PRETTY_FUNCTION__);
}

GLuint Texture::getID()    
{
    return _id; 
}

Texture::Channels Texture::getChannels()
{
    return _channels;
}

void Texture::setChannels(Texture::Channels channels)
{
    _channels = channels;
}

Texture2D::Texture2D(Texture::Channels channels, int width, int height)
    : Texture(channels), _width(width), _height(height)
{
}

Texture2D::~Texture2D()
{
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
    glBindTexture(GL_TEXTURE_2D, _id);
    getGLError(__PRETTY_FUNCTION__);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if(getChannels() == DEPTH) {
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
}

void Texture2D::release()    
{
    glBindTexture(GL_TEXTURE_2D, 0);
    getGLError(__PRETTY_FUNCTION__);
}

void Texture2D::create(void* data)
{
    GLenum _format, _internalFormat, _type;
    switch(getChannels()) {
        case RGB:
            _format = GL_RGB;
            _internalFormat = GL_RGB;
            _type = GL_UNSIGNED_BYTE;
            break;
        case RGBA:
            _format = GL_RGBA;
            _internalFormat = GL_RGBA;
            _type = GL_UNSIGNED_BYTE;
            break;
        case DEPTH:
            _format = GL_DEPTH_COMPONENT24;
            _internalFormat = GL_DEPTH_COMPONENT;
            _type = GL_UNSIGNED_BYTE;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 _format,
                 _width,
                 _height,
                 0,
                 _internalFormat,
                 _type,
                 data);
    getGLError(__PRETTY_FUNCTION__);
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
    auto format = QGLFormat::defaultFormat();
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
