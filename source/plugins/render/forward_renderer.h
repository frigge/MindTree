#ifndef MT_GL_FORWARD_RENDERER_H
#define MT_GL_FORWARD_RENDERER_H

#include "render_setup.h"

namespace MindTree {

namespace GL {

class ForwardRenderer : public RenderConfigurator
{
public:
    ForwardRenderer(std::shared_ptr<Camera> camera, Widget3DManager *manager=nullptr);

    void setGeometry(std::shared_ptr<Group> grp);

private:
    void setupDefaultLights();

    uint _maxLightCount;
};

}
}

#endif
