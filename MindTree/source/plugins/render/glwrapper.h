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

class VBO;
class IBO;

class VAO
{
public:
    VAO();
    virtual ~VAO();

    void bind();
    void release();
    //void addData(std::shared_ptr<VertexList> l);
    void setPolygons(std::shared_ptr<PolygonList> l);

private:
    //std::vector<std::shared_ptr<VBO>> vbos;
    //std::shared_ptr<IBO> ibo;

    GLuint id;
    bool bound;
};

class VBO
{
public:
    VBO(std::string name, GLuint index);
    virtual ~VBO();

    std::string getName();
    void bind();
    void release();
    void data(std::shared_ptr<VertexList> l);
    GLint getIndex();

private:
    GLuint id, index;
    std::string name;
};

class IBO
{
public:
    IBO();
    virtual ~IBO();

    void bind();
    void release();
    void data(std::shared_ptr<PolygonList> l);

private:
    GLuint id;
};

class UBO
{
public:
    UBO();
    void bind();
    void release();
    virtual ~UBO();

private:
    GLuint id;
};

class Texture;
class FBO
{
public:
    FBO();
    virtual ~FBO();

    void bind();
    void release();

    void attachColorTexture(std::shared_ptr<Texture> tex);
    void attachDepthTexture(std::shared_ptr<Texture> tex);

private:
    uint color_attachements;
    GLuint id;
};

class ShaderProgram
{
public:
    ShaderProgram();
    virtual ~ShaderProgram();

    void bind();
    void release();

    void link();
    void addShaderFromSource(std::string src, GLenum type);
    void addShaderFromFile(std::string filename, GLenum type);

    void setUniform(std::string name, const glm::vec3 &value);
    void setUniform(std::string name, const glm::vec4 &value);
    void setUniform(std::string name, float value);
    void setUniform(std::string name, int value);
    void setUniform(std::string name, const glm::mat4 &value);

    void bindAttributeLocation(unsigned int index, std::string name);
    void vertexAttribute(std::string name, const std::vector<int> &data);
    void vertexAttribute(std::string name, const std::vector<glm::vec3> &data);
    bool hasAttribute(std::string name);

    void bufferedAttribute(std::string name);

    void enableAttribute(std::string name);
    void disableAttribute(std::string name);

private:
    GLuint id;
    bool isBound;
    int attributes = 0;
    std::size_t offset = 0;
    std::unordered_map<std::string, int> attributeLocations;
};

class Texture
{
public:
    Texture();
    virtual ~Texture();
    void bind();
    void release();
    GLuint getID();
    void createTexture(uint width, uint height);

private:
    GLuint id;
};

class ResourceManager
{
public:
    ResourceManager();
    virtual ~ResourceManager();
    std::shared_ptr<VBO> createVBO(std::shared_ptr<MeshData> data, std::string name);
    std::shared_ptr<VBO> getVBO(std::shared_ptr<MeshData> data, std::string name);

    void scheduleCleanUp(std::unique_ptr<ShaderProgram> prog);
    void scheduleCleanUp(std::unique_ptr<VAO> prog);

private:
    friend class RenderManager;

    void cleanUp();
    std::vector<std::unique_ptr<ShaderProgram>> _scheduledShaders;
    std::vector<std::unique_ptr<VAO>> _scheduledVAOs;
    std::unordered_map<std::shared_ptr<MeshData>, std::vector<std::shared_ptr<VBO>>> vbo_map;
    std::unordered_map<std::shared_ptr<MeshData>, std::vector<std::shared_ptr<IBO>>> ibo_map;
    std::unordered_map<std::shared_ptr<MeshData>, std::vector<std::shared_ptr<VBO>>> ubo_map;
};

class QtContext : public QGLContext
{
public:
    QtContext();
    virtual ~QtContext();

    static QtContext* getContext();
    std::shared_ptr<ResourceManager> getManager();

    void makeCurrent();
    void doneCurrent();
    static QtContext* getCurrent();

private:
    static QtContext* currentContext;
    std::shared_ptr<ResourceManager> manager;
    QGLFormat format;
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: GLWRAPPER_TLVMZFDN */
