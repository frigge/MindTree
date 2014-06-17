#ifndef MT_PRIMITIVE_RENDERER_H
#define MT_PRIMITIVE_RENDERER_H

#include "glwrapper.h"
#include "render.h"

namespace MindTree {

namespace GL {

class PrimitiveRenderer : public Renderer
{
public:
    PrimitiveRenderer();
    virtual ~PrimitiveRenderer();

protected:
    virtual void init();
    virtual void initVAO();

    std::shared_ptr<VBO> _vbo;
};

class LineRenderer : public PrimitiveRenderer
{
public:
    std::shared_ptr<ShaderProgram> getProgram();

private:
    static std::shared_ptr<ShaderProgram> _defaultProgram;
};

class QuadRenderer : public PrimitiveRenderer
{
public:
    std::shared_ptr<ShaderProgram> getProgram();

private:
    static std::shared_ptr<ShaderProgram> _defaultProgram;
};

class FullscreenQuadRenderer : public PrimitiveRenderer
{
public:
    FullscreenQuadRenderer(RenderPass *pass);
    ~FullscreenQuadRenderer();

    std::shared_ptr<ShaderProgram> getProgram();

protected:
    void init();
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    RenderPass *_pass;
    static std::shared_ptr<ShaderProgram> _defaultProgram;
    std::shared_ptr<VBO> _coord_vbo;
};

class GridRenderer : public PrimitiveRenderer
{
public:
    GridRenderer(int width, int height, int xres, int yres);
    virtual ~GridRenderer();

    std::shared_ptr<ShaderProgram> getProgram();

    void setColor(glm::vec4 color);
    void setThickness(float thickness);

protected:
    void init();
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    int _width, _height, _xres, _yres;
    float _thickness;
    glm::vec4 _color;
    static std::shared_ptr<ShaderProgram> _defaultProgram;
};

}
}

#endif
