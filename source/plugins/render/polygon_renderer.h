#ifndef MT_GL_POLYGONRENDERER_H
#define MT_GL_POLYGONRENDERER_H
#include "cstdint"
#include "geoobject_renderer.h"

namespace MindTree
{
namespace GL
{

class PolygonRenderer : public GeoObjectRenderer
{
public:
    PolygonRenderer(std::shared_ptr<GeoObject> o);
    virtual ~PolygonRenderer();

    ShaderProgram* getProgram();

protected:
    void draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program);

private:
    std::vector<uint> triangulate();

    size_t _triangleCount;
    void initCustom();
    ResourceHandle<VBO> _polyColors;
    ResourceHandle<IBO> _triangulatedIBO;
    ResourceHandle<Texture> _polyColorTexture;
};

class EdgeRenderer : public GeoObjectRenderer
{
public:
    EdgeRenderer(std::shared_ptr<GeoObject> o);
    virtual ~EdgeRenderer();

    ShaderProgram* getProgram();

protected:
    void draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program);

private:
    void initCustom();
};

class PointRenderer : public GeoObjectRenderer
{
public:
    PointRenderer(std::shared_ptr<GeoObject> o);
    virtual ~PointRenderer();

    ShaderProgram* getProgram();

protected:
    void draw(const CameraPtr &camera, const RenderConfig &config, ShaderProgram* program);
};

}
}

#endif
