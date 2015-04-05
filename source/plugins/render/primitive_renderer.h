#ifndef MT_PRIMITIVE_RENDERER_H
#define MT_PRIMITIVE_RENDERER_H

#include "glwrapper.h"
#include "render.h"

namespace MindTree { 
namespace GL {

class RenderPass;

class ShapeRendererGroup : public Renderer
{
public:
    ShapeRendererGroup(ShapeRendererGroup *parent=nullptr);

    virtual ~ShapeRendererGroup() {}

    std::shared_ptr<ShaderProgram> getProgram();

    virtual void setFillColor(glm::vec4 color);
    virtual void setBorderColor(glm::vec4 color);
    glm::vec4 getFillColor()const;
    glm::vec4 getBorderColor()const;

    void setBorderWidth(float border);

    float getBorderWidth()const;
    void setFixedScreenSize(bool fixed);
    bool getFixedScreenSize()const;
    void setScreenOriented(bool orient);
    bool getScreenOriented()const;

    void setParentPrimitive(ShapeRendererGroup *renderer);
    void setChildPrimitive(ShapeRendererGroup *renderer);

    void setStaticTransformation(glm::mat4 trans);
    void staticTransform(glm::mat4 trans);
    glm::mat4 getStaticWorldTransformation()const;

protected:
    virtual void init(std::shared_ptr<ShaderProgram> prog);
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

    static std::weak_ptr<ShaderProgram> _defaultProgram;

private:
    std::vector<ShapeRendererGroup*> _childPrimitives;

    glm::mat4 _staticTransformation;
    glm::vec4 _fillColor, _borderColor;

    mutable std::mutex _fillColorLock;
    mutable std::mutex _borderColorLock;
    mutable std::mutex _staticTransformationLock;

    std::atomic<bool> _fixedScreenSize, _screenOriented;
    std::atomic<float> _borderWidth;

};

class ShapeRenderer : public ShapeRendererGroup
{
public:
    virtual ~ShapeRenderer();

protected:
    ShapeRenderer(ShapeRendererGroup *parent=nullptr) : ShapeRendererGroup(parent) {}

    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
};

class LineRenderer : public ShapeRenderer
{
public:
    LineRenderer(ShapeRendererGroup *parent=nullptr) : ShapeRenderer(parent) {}
    LineRenderer(std::initializer_list<glm::vec3> points);
    ~LineRenderer();

    void setPoints(std::initializer_list<glm::vec3> points);

protected:
    virtual void init(std::shared_ptr<ShaderProgram> prog);
    virtual void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    VertexList _points;
    std::shared_ptr<VBO> _vbo;
};

class QuadRenderer : public ShapeRenderer
{
public:
    QuadRenderer(float width, float height, ShapeRendererGroup *parent=nullptr);

protected:
    void init(std::shared_ptr<ShaderProgram> prog);

private:
    float _width, _height;
    std::shared_ptr<VBO> _vbo;
};

class GridRenderer : public ShapeRenderer
{
public:
    GridRenderer(int width, int height, int xres, int yres, ShapeRendererGroup *parent=nullptr);
    virtual ~GridRenderer();

    void setAlternatingColor(glm::vec4 col);

protected:
    void init(std::shared_ptr<ShaderProgram> prog);
    virtual void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    std::shared_ptr<ShaderProgram> getProgram();

private:
    int _width, _height, _xres, _yres;
    glm::vec4 _alternatingColor;
    static std::weak_ptr<ShaderProgram> _defaultProgram;
    std::shared_ptr<VBO> _vbo;
};

class DiscRenderer : public ShapeRenderer
{
public:
    DiscRenderer(ShapeRendererGroup *parent=nullptr);

protected:
    void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void init(std::shared_ptr<ShaderProgram> prog);

private:
    int _segments;
    static std::shared_ptr<VBO> _vbo;
};

class CircleRenderer : public ShapeRenderer
{
public:
    CircleRenderer(ShapeRendererGroup *parent=nullptr);

protected:
    void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void init(std::shared_ptr<ShaderProgram> prog);

private:
    int _segments;
    static std::shared_ptr<VBO> _vbo;
};

class ConeRenderer : public ShapeRenderer
{
public:
    ConeRenderer(ShapeRendererGroup *parent=nullptr);

protected:
    void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    virtual void init(std::shared_ptr<ShaderProgram> prog);

private:
    int _segments;
    static std::shared_ptr<VBO> _vbo;
};

class SphereRenderer : public ShapeRenderer
{
public:
    SphereRenderer(int u=8, int v=8);

protected:
    void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void init(std::shared_ptr<ShaderProgram> prog);

private:
    int _u_segments, _v_segments;
    static std::shared_ptr<VBO> _vbo;
    static std::shared_ptr<IBO> _ibo;
};

class SinglePointRenderer : public ShapeRenderer
{
public:
    SinglePointRenderer(ShapeRendererGroup *parent=nullptr);
    void setPointSize(int size);
    int getPointSize() const;
    void setPosition(glm::vec3 position);

protected:
    void drawBorder(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void drawFill(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);
    void init(std::shared_ptr<ShaderProgram> prog);

private:
    int _pointSize;
    static std::shared_ptr<VBO> _vbo;
};

class ArrowRenderer : public ShapeRendererGroup
{
public:
    ArrowRenderer(ShapeRendererGroup *parent=nullptr);
    void setFillColor(glm::vec4 color);
    void setBorderColor(glm::vec4 color);
};

}
}

#endif
