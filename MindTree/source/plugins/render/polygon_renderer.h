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

    std::shared_ptr<ShaderProgram> getProgram();

protected:
    void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    void initCustom();
    static std::weak_ptr<ShaderProgram> _defaultProgram;
};

class EdgeRenderer : public GeoObjectRenderer
{
public:
    EdgeRenderer(std::shared_ptr<GeoObject> o);
    virtual ~EdgeRenderer();

    std::shared_ptr<ShaderProgram> getProgram();

protected:
    void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    void initCustom();
    static std::weak_ptr<ShaderProgram> _defaultProgram;
};

class PointRenderer : public GeoObjectRenderer
{
public:
    PointRenderer(std::shared_ptr<GeoObject> o);
    virtual ~PointRenderer();

    std::shared_ptr<ShaderProgram> getProgram();

protected:
    void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    static std::weak_ptr<ShaderProgram> _defaultProgram;
};

}
}

#endif
