#ifndef MT_VIEWPORT_WIDGETS
#define MT_VIEWPORT_WIDGETS

#include "../render/primitive_renderer.h"
#include "data/signal.h"
#include "graphics/shapes.h"
#include "data/nodes/data_node_socket.h"
#include "interactive.h"
#include "memory"

namespace MindTree {
namespace GL {
class RenderPass;
}

class Widget3D
{
    typedef std::function<std::unique_ptr<Widget3D>()> Factory_t;
public:
    Widget3D(MindTree::SocketType type);
    Widget3D(const Widget3D &other) = delete;

	bool checkMousePressed(const std::shared_ptr<Camera> &cam,
						   glm::ivec2 pixel,
						   glm::vec3 *hitpoint,
	                       float *depth);

	bool checkMouseMoved(const std::shared_ptr<Camera> &cam,
                         glm::ivec2 pixel,
						 glm::vec3 *hitpoint,
                         float *depth);

    void toggleVisible();
    MindTree::GL::ShapeRendererGroup* renderer();

    void pressMouse(glm::vec3 point);
    void moveMouse(glm::vec3 point);
    void enterHover(glm::vec3 point);
    void leaveHover();
    void forceMouseReleased();

    static void registerWidget(Factory_t factory);

protected:
    virtual MindTree::GL::ShapeRendererGroup* createRenderer() = 0;
    void setVisible(bool visible);

    virtual void mousePressed(glm::vec3 point);
    virtual void mouseMoved(glm::vec3 point);
    virtual void mouseReleased(glm::vec3 point);
    virtual void mouseDraged(glm::vec3 point);

    virtual bool hoverEntered(glm::vec3 point);
    virtual void hoverLeft();

    MindTree::DNode *_node;
    MindTree::GL::ShapeRendererGroup *_renderer;
    MindTree::Signal::CallbackVector _callbacks;

    glm::vec3 startPoint;
    glm::vec4 _hoverBorderColor, _hoverFillColor, _outBorderColor, _outFillColor;
	Interactive::ShapeGroup shapegroup_;
    bool _visible, _hover, _pressed, _screenOriented, _screenSize;

private:
	glm::mat4 computeTransformation(const std::shared_ptr<Camera> &cam) const;

    virtual void update();
    void updateTransformation();
    void setNode(MindTree::DNode *node);
	bool intersectShapes(const std::shared_ptr<Camera> &cam,
						 glm::ivec2 pixel,
						 float *depth,
	                     glm::vec3 *hitpoint);

	MindTree::SocketType _type;

    float _size;

    static std::vector<Factory_t> _widget_factories;
    friend class Widget3DManager;
};

class Widget3DManager
{
public:
    Widget3DManager();

    void insertWidgetsIntoRenderPass(MindTree::GL::RenderPass *pass);
    bool mousePressEvent(const CameraPtr &cam, glm::ivec2 pos);
    bool mouseMoveEvent(const CameraPtr &cam, glm::ivec2 pos);
    void mouseReleaseEvent();

private:
    std::vector<std::unique_ptr<Widget3D>> _widgets;
    Widget3D *_clicked_widget{nullptr};

};
}
#endif
