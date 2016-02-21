#ifndef MT_VIEWPORT_WIDGETS
#define MT_VIEWPORT_WIDGETS

#include "../render/primitive_renderer.h"
#include "data/signal.h"
#include "graphics/shapes.h"
#include "data/nodes/nodetype.h"
#include "memory"

class Widget3D;
namespace MindTree {
namespace GL {
class RenderPass;
}
}

typedef std::shared_ptr<Widget3D> Widget3DPtr;
class Widget3D 
{
    typedef std::function<Widget3DPtr()> Factory_t;
public:
    Widget3D(MindTree::NodeType type);
    Widget3D(const Widget3D &other);
    virtual ~Widget3D();

    bool checkMousePressed(const std::shared_ptr<Camera> cam, glm::ivec2 pixel, glm::ivec2 viewportSize, float *depth=nullptr);
    bool checkMouseMoved(const std::shared_ptr<Camera> cam, glm::ivec2 pixel, glm::ivec2 viewportSize, float *depth=nullptr);
    bool checkMouseReleased(const std::shared_ptr<Camera> cam, glm::ivec2 pixel, glm::ivec2 viewportSize);

    void toggleVisible();
    virtual MindTree::GL::ShapeRendererGroup* createRenderer() = 0;
    MindTree::GL::ShapeRendererGroup* renderer();
    void addShape(std::shared_ptr<Shape> shape);

    void forceHoverLeft();
    void forceMouseReleased();

    static void registerWidget(Factory_t factory);

protected:
    void setVisible(bool visible);

    virtual void mousePressed(glm::vec3 point);
    virtual void mouseMoved(glm::vec3 point);
    virtual void mouseReleased(glm::vec3 point);
    virtual void mouseDraged(glm::vec3 point);

    virtual void hoverEntered(glm::vec3 point);
    virtual void hoverLeft();

    MindTree::DNode *_node;
    MindTree::GL::ShapeRendererGroup *_renderer;
    MindTree::Signal::CallbackVector _callbacks;
    std::vector<std::shared_ptr<Shape>> _shapes;

    glm::vec3 startPoint;
    glm::vec4 _hoverBorderColor, _hoverFillColor, _outBorderColor, _outFillColor;

    bool _screenOriented, _screenSize;

private:
    virtual void update();
    void updateTransformation();
    void setNode(MindTree::DNode *node);
    bool intersectShapes(const std::shared_ptr<Camera> &cam,
                         glm::ivec2 pixel,
                         glm::ivec2 viewportSize,
                         glm::vec3 *hitpoint);

    MindTree::NodeType _type;
    bool _visible, _hover, _pressed;

    float _size;
    
    static std::vector<Factory_t> _widget_factories;
    friend class Widget3DManager;
};

class Widget3DManager
{
public:
    Widget3DManager();

    void insertWidgetsIntoRenderPass(MindTree::GL::RenderPass *pass);
    bool mousePressEvent(CameraPtr cam, glm::ivec2 pos, glm::ivec2 viewportSize);
    bool mouseMoveEvent(CameraPtr cam, glm::ivec2 pos, glm::ivec2 viewportSize);
    void mouseReleaseEvent();

private:
    std::vector<Widget3DPtr> _widgets;

};

class TranslateWidget : public Widget3D {
public:
    enum Axis {
        X, Y, Z, XY, XZ, YZ
    };

    TranslateWidget(Axis axis);
    virtual ~TranslateWidget();

protected:
    void mouseDraged(glm::vec3 point);

private:
    Axis _axis;
};
#endif
