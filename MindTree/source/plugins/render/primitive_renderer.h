#ifndef MT_PRIMITIVE_RENDERER_H
#define MT_PRIMITIVE_RENDERER_H

#include "glwrapper.h"
#include "render.h"

namespace MindTree {

namespace GL {

class RenderPass;

class PrimitiveRenderer : public Renderer
{
public:
    PrimitiveRenderer();
    virtual ~PrimitiveRenderer();

    std::shared_ptr<ShaderProgram> getProgram();

    void setFillColor(glm::vec4 color);
    void setBorderColor(glm::vec4 color);
    void setBorderWidth(float border);
    float getBorderWidth();
    void setFixedScreenSize(bool fixed);
    bool getFixedScreenSize();
    void setScreenOriented(bool orient);
    bool getScreenOriented();

    void setParentPrimitive(PrimitiveRenderer *renderer);
    void setChildPrimitive(PrimitiveRenderer *renderer);

    void setStaticTransformation(glm::mat4 trans);
    glm::mat4 getStaticWorldTransformation();

protected:
    virtual void init();
    virtual void initVAO();
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

    std::shared_ptr<VBO> _vbo;

    glm::mat4 _staticTransformation;

private:
    static std::shared_ptr<ShaderProgram> _defaultProgram;
    glm::vec4 _fillColor, _borderColor;
    bool _fixedScreenSize, _screenOriented;
    float _borderWidth;
    std::vector<PrimitiveRenderer*> _childPrimitives;
};

class LineRenderer : public PrimitiveRenderer
{
public:
    LineRenderer();
    LineRenderer(std::initializer_list<glm::vec3> points);
    ~LineRenderer();

    void setPoints(std::initializer_list<glm::vec3> points);

protected:
    virtual void init();
    virtual void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    VertexList _points;
};

class Widget3dRenderer : public PrimitiveRenderer
{
public:
    Widget3dRenderer(); virtual ~Widget3dRenderer();

protected:
    virtual void init();
    virtual void initVAO();
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
};

class QuadRenderer : public PrimitiveRenderer
{
public:
    QuadRenderer(float width, float height);

protected:
    void init();

private:
    float _width, _height;
};

class FullscreenQuadRenderer : public Renderer
{
public:
    FullscreenQuadRenderer(RenderPass *pass);
    virtual ~FullscreenQuadRenderer();

    std::shared_ptr<ShaderProgram> getProgram();

protected:
    virtual void initVAO();
    virtual void init();
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    RenderPass *_pass;
    static std::shared_ptr<ShaderProgram> _defaultProgram;
    std::shared_ptr<VBO> _vbo;
    std::shared_ptr<VBO> _coord_vbo;
};

class GridRenderer : public PrimitiveRenderer
{
public:
    GridRenderer(int width, int height, int xres, int yres);
    virtual ~GridRenderer();

    void setAlternatingColor(glm::vec4 col);

protected:
    void init();
    virtual void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    std::shared_ptr<ShaderProgram> getProgram();

private:
    int _width, _height, _xres, _yres;
    glm::vec4 _alternatingColor;
    static std::shared_ptr<ShaderProgram> _defaultProgram;
};

class ConeRenderer : public PrimitiveRenderer
{
public:
    ConeRenderer(float height, float radius, int segments);
    ~ConeRenderer();

protected:
    void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void init();

private:
    float _height, _radius;
    int _segments;
};

}
}

#endif
