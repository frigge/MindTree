#include "GL/glew.h"
#include "QGLContext"
#include "glm/gtc/type_ptr.hpp"
#include "iostream"
#include "fstream"
#include "glwrapper.h"

using namespace MindTree::GL;

VAO::VAO()
    : bound(false)
{
    glGenVertexArrays(1, &id);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &id);
}

void VAO::bind()    
{
    glBindVertexArray(id);
    //for(auto &vbo : vbos) vbo->bind();
    bound = true;
}

void VAO::release()    
{
    glBindVertexArray(0);
    //for(auto &vbo : vbos) vbo->release();
    bound = false;
}

//void VAO::addData(std::string name, std::shared_ptr<VertexList> l)
//{
//    auto vbo = QtContext::getCurrent()->getManager()->createVBO(name);
//    vbo->bind();
//    vbo->data(l);
//    glEnableVertexAttribArray(index);
//    vbo->release();
//}

void VAO::setPolygons(std::shared_ptr<PolygonList> l)    
{
    //ibo = std::make_shared<IBO>();
    //ibo->bind();
    //ibo->data(l);
    //ibo->release();
}

VBO::VBO(std::string name, GLuint index)
    : name(name), index(index)
{
    glGenBuffers(1, &id);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &id);
}

std::string VBO::getName()    
{
    return name;
}

void VBO::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void VBO::release()    
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::data(std::shared_ptr<VertexList> l)
{
    glBufferData(GL_ARRAY_BUFFER, l->size() * 3 * sizeof(float), &(*l)[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(index);
}

GLint VBO::getIndex()    
{
    return index;
}

IBO::IBO()
{
    glGenBuffers(1, &id);
}

IBO::~IBO()
{
    glDeleteBuffers(1, &id);
}

void IBO::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IBO::release()    
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::data(std::shared_ptr<PolygonList> l)
{
    int size = 0;
    
    //find out size of index buffer by adding together polygon sizes
    for(const auto &p : *l){
        size += p.size();
    }

    //populate index array
    std::vector<int> d;
    d.resize(size);
    for(const auto &p : *l){
        d.assign(begin(p.verts()), end(p.verts()));
    }
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(int), &d[0], GL_STATIC_DRAW);
}

UBO::UBO()
{
    glGenBuffers(1, &id);
}

UBO::~UBO()
{
    glDeleteBuffers(1, &id);
}

void UBO::bind()
{
    glBindBuffer(GL_UNIFORM_BUFFER, id);
}

void UBO::release()    
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

FBO::FBO()
    : color_attachements(0)
{
    glGenFramebuffers(1, &id);
}

FBO::~FBO()
{
    glDeleteFramebuffers(1, &id);
}

void FBO::bind()    
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FBO::release()    
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::attachColorTexture(std::shared_ptr<Texture> tex)    
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + color_attachements++,
        GL_TEXTURE_2D,
        tex->getID(),
        0);
}

void FBO::attachDepthTexture(std::shared_ptr<Texture> tex)    
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        tex->getID(),
        0);
}

ShaderProgram::ShaderProgram()
    : isBound(false)
{
    id = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}

void ShaderProgram::bind()
{
    isBound = true;
    glUseProgram(id);
}

void ShaderProgram::release()    
{
    isBound = false;
    glUseProgram(0);
}

void ShaderProgram::link()    
{
    glLinkProgram(id);
}

void ShaderProgram::addShaderFromSource(std::string src, GLenum type)    
{
    GLuint shader = glCreateShader(type);
    const char* c_str = src.c_str();
    glShaderSource(shader, 1, &c_str, 0);

    glCompileShader(shader);
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

    glAttachShader(id, shader);
}

void ShaderProgram::addShaderFromFile(std::string filename, GLenum type)
{
    std::ifstream stream(filename);

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
    GLint location = glGetUniformLocation(id, name.c_str());
    if(location > -1) glUniform3f(location, value.x, value.y, value.z);
}

void ShaderProgram::setUniform(std::string name, const glm::vec4 &value)    
{
    GLint location = glGetUniformLocation(id, name.c_str());
    if(location > -1) glUniform4f(location, value.x, value.y, value.z, value.w);
}

void ShaderProgram::setUniform(std::string name, float value)    
{
    GLint location = glGetUniformLocation(id, name.c_str());
    if(location > -1) glUniform1f(location, value);
}

void ShaderProgram::setUniform(std::string name, int value)    
{
    GLint location = glGetUniformLocation(id, name.c_str());
    if(location > -1) glUniform1i(location, value);
}

void ShaderProgram::setUniform(std::string name, const glm::mat4 &value)    
{
    GLint location = glGetUniformLocation(id, name.c_str());
    if(location > -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::bindAttributeLocation(unsigned int index, std::string name)    
{
    bool wasntbound = false;
    if(!isBound) {
        wasntbound = true;
        bind();
    }
    glBindAttribLocation(id, index, name.c_str()); 
    std::cout << "bound attribute: " << name << " to location: " << index << std::endl;
    //
    //needs to be relinked so that the binding actually goes into effect
    link();
    if(wasntbound) release();
}

void ShaderProgram::vertexAttribute(std::string name, const std::vector<glm::vec3> &data)    
{
    if(!attributeLocations.count(name))
        attributeLocations[name] = attributes++;
    uint index = attributeLocations[name];
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, offset, &data[0][0]);
    bindAttributeLocation(index, name);
    offset += sizeof(glm::vec3) * data.size();
}

void ShaderProgram::vertexAttribute(std::string name, const std::vector<int> &data)    
{
    if(!attributeLocations.count(name))
        attributeLocations[name] = attributes++;
    uint index = attributeLocations[name];
    glVertexAttribPointer(id, 1, GL_INT, GL_FALSE, offset, &data[0]);
    bindAttributeLocation(attributeLocations[name], name);
    offset += sizeof(int) * data.size();
}

bool ShaderProgram::hasAttribute(std::string name)    
{
    bool wasntbound = false;
    if(!isBound) {
        wasntbound = true;
        bind();
    }
    GLint location = glGetAttribLocation(id, name.c_str());
    std::cout << "location of " << name << ": " << location << std::endl;
    if(wasntbound) release();

    return location > -1;
}

void ShaderProgram::enableAttribute(std::string name)    
{
    glEnableVertexAttribArray(attributeLocations[name]);
}

void ShaderProgram::disableAttribute(std::string name)    
{
    glDisableVertexAttribArray(attributeLocations[name]);
}

Texture::Texture()
{
    glGenTextures(1, &id);
}

Texture::~Texture()
{
    glDeleteTextures(1, &id); 
}

void Texture::bind()    
{
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::release()    
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::getID()    
{
    return id; 
}

void Texture::createTexture(uint width, uint height)    
{
    
}

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

std::shared_ptr<VBO> ResourceManager::createVBO(std::shared_ptr<MeshData> data, std::string name)    
{
    auto &vbos = vbo_map[data];
    uint index = vbos.size();
    std::cout << "creating new VBO for variable: " << name << " to index: " << index << std::endl;
    auto vbo = std::make_shared<VBO>(name, index);
    vbos.push_back(vbo);
    return vbo;
}

std::shared_ptr<VBO> ResourceManager::getVBO(std::shared_ptr<MeshData> data, std::string name)    
{
    auto &vbos = vbo_map[data];
    for(auto vbo : vbos)
       if(vbo->getName() == name)
          return vbo; 

    auto vbo = createVBO(data, name);
    return vbo;
}

QtContext* QtContext::currentContext=nullptr;

QtContext::QtContext()
    : QGLContext(QGLFormat::defaultFormat()),
      manager(std::make_shared<ResourceManager>())
{
    auto format = QGLFormat::defaultFormat();
    //format.setVersion(3, 2);
    //format.setProfile(QGLFormat::CoreProfile);
    setFormat(format);
}

QtContext::~QtContext()
{
}

QtContext* QtContext::getContext()    
{
    return new QtContext();
}

void QtContext::makeCurrent()    
{
    QGLContext::makeCurrent();
    currentContext = this;
}

void QtContext::doneCurrent()    
{
    QGLContext::doneCurrent();
    currentContext = nullptr;
}

QtContext* QtContext::getCurrent()    
{
    return currentContext;
}

std::shared_ptr<ResourceManager> QtContext::getManager()    
{
    return manager;
}
