#ifndef RENDER_GR953KUN

#define RENDER_GR953KUN

#include "memory"
#include "mutex"
#include "vector"
#include "../datatypes/Object/object.h"
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
    void draw(const CameraPtr camera, const RenderConfig &config);

    void drawPoints(const CameraPtr camera);
    void drawEdges(const CameraPtr camera);
    void drawPolygons(const CameraPtr camera, const RenderConfig &config);
    void drawVertexNormals(const CameraPtr camera);
    void drawFaceNormals(const CameraPtr camera);

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

    void draw(const CameraPtr camera, const RenderConfig &config);

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
    void render(const RenderConfig &config);

    void setGeometry(std::shared_ptr<Group> g);
    void setCamera(CameraPtr camera);
    CameraPtr getCamera();

    void setSize(int width, int height);

private:
    int _width, _height;
    bool _viewportChanged;
    std::shared_ptr<RenderGroup> group;
    std::shared_ptr<Camera> _camera;
    std::shared_ptr<FBO> target;
    std::mutex _sizeLock;
    std::mutex _geometryLock;
};

} /* GL */
} /* MindTree */

#endif /* end of include guard: RENDER_GR953KUN */
