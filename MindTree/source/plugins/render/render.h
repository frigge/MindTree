#ifndef RENDER_GR953KUN

#define RENDER_GR953KUN

#include "memory"
#include "vector"
#include "glm/glm.hpp"

class GeoObject;
namespace MindTree
{
namespace GL
{

class VAO;
class FBO;

class ShaderProgram;

class RenderConfig;

class Render
{
public:
    Render(std::shared_ptr<GeoObject> o);
    virtual ~Render();

    virtual void init();
    void draw(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config);

    void drawPoints(const glm::mat4 &view, const glm::mat4 &projection);
    void drawEdges(const glm::mat4 &view, const glm::mat4 &projection);
    void drawPolygons(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config);
    void drawVertexNormals(const glm::mat4 &view, const glm::mat4 &projection);
    void drawFaceNormals(const glm::mat4 &view, const glm::mat4 &projection);

    void setPointProgram(ShaderProgram *prog);
    void setEdgeProgram(ShaderProgram *prog);
    void setPolyProgram(ShaderProgram *prog);

protected:
    std::shared_ptr<GeoObject> obj;
    std::unique_ptr<VAO> vao;
    std::unique_ptr<ShaderProgram> pointProgram;
    std::unique_ptr<ShaderProgram> edgeProgram;
    std::unique_ptr<ShaderProgram> polyProgram;
    std::vector<uint> polysizes;
    std::vector<const uint*> polyindices;
    std::vector<uint> triangles;

private:
    void setUniforms(ShaderProgram *prog);

    bool initialized;
};

class MeshRender : public Render
{
public:
    MeshRender(std::shared_ptr<GeoObject> o);
    virtual ~MeshRender();

    virtual void init();
    void generateIndices();
    void tesselate();

    void initPointProgram();
    void initEdgeProgram();
    void initPolyProgram();

private:
};

class RenderGroup
{
public:
    RenderGroup(std::shared_ptr<Group> g);
    virtual ~RenderGroup();

    void draw(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config);

private:
    void addObject(std::shared_ptr<GeoObject> obj);

    std::shared_ptr<Group> group;
    std::vector<std::unique_ptr<Render>> renders;
};

class RenderPass
{
public:
    RenderPass();
    virtual ~RenderPass();
    void render(const glm::mat4 &view, const glm::mat4 &projection, const RenderConfig &config);

    void setGeometry(std::shared_ptr<Group> g);

private:
    std::shared_ptr<RenderGroup> group;
    std::shared_ptr<FBO> target;
};

class RenderConfig
{
public:
    void setDrawPoints(bool draw);
    void setDrawEdges(bool draw);
    void setDrawPolygons(bool draw);
    void setShowFlatShaded(bool b);
    bool drawPoints() const;
    bool drawEdges() const;
    bool drawPolygons() const;
    bool flatShading() const;

private:
    bool _drawPoints = true;
    bool _drawEdges = true;
    bool _drawPolygons = true;
    bool _flatShading = false;
};

class RenderManager
{
public:
    RenderManager();
    virtual ~RenderManager();

    void init();

    RenderPass* addPass();
    void removePass(uint index);
    void draw(const glm::mat4 &view, const glm::mat4 &projection);
    RenderPass* getPass(uint index);
    void setConfig(RenderConfig cfg);
    RenderConfig getConfig();

private:
    glm::vec4 backgroundColor;
    std::list<std::unique_ptr<RenderPass>> passes;
    RenderConfig config;
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: RENDER_GR953KUN */
