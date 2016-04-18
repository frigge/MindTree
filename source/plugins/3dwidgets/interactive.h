#ifndef MT_GL_WIDGETS_INTERACTIVE_H
#define MT_GL_WIDGETS_INTERACTIVE_H

#define GLM_SWIZZLE

#include <memory>
#include <glm/glm.hpp>
#include "graphics/shapes.h"
#include "data/raytracing/ray.h"
#include "../render/primitive_renderer.h"

class Camera;

namespace MindTree {
namespace Interactive {

class ShapeGroup
{
public:
    ShapeGroup();

    virtual bool intersect(const std::shared_ptr<Camera> &cam,
                           glm::ivec2 pixel,
                           glm::ivec2 viewportSize,
                           glm::vec3 *hitpoint=nullptr) const;

    virtual GL::ShapeRendererGroup* createRenderer()
    {
        renderer_ = new GL::ShapeRendererGroup();
        return renderer_;
    }

    glm::mat4 computeTransformation(const std::shared_ptr<Camera> &cam,
                                    glm::ivec2 pixel,
                                    glm::ivec2 viewportSize) const;

    inline void setScreenOriented(bool orient)
    {
        screenOriented_ = orient;
    }

    inline void setScreenSize(bool screen)
    {
        screenSize_ = screen;
    }

protected:
    bool screenOriented_, screenSize_;

    GL::ShapeRendererGroup *renderer_;
    std::vector<std::unique_ptr<ShapeGroup>> children_;
};

template<typename T> struct ShapeRendererTrait {
    GL::ShapeRendererGroup *createRenderer()
    {
        return nullptr;
    }
};

template<typename T>
class Shape : public ShapeGroup
{
    bool intersect(const std::shared_ptr<Camera> &cam,
                   glm::ivec2 pixel,
                   glm::ivec2 viewportSize,
                   glm::vec3 *hitpoint=nullptr) const override
    {
        auto camPos = cam->getPosition();
        auto view = cam->getViewMatrix();
        auto projection = cam->getProjection();
        auto viewProj = projection * view;
        Ray r = Ray::primaryRay(viewProj, camPos, pixel, viewportSize);
        auto finalTransform = computeTransformation(cam, pixel, viewportSize);

        return r.intersect(finalTransform * shape_, hitpoint);
    }

    GL::ShapeRendererGroup *createRenderer() override
    {
        return ShapeRendererTrait<T>::createRenderer();
    }

private:
    T shape_;
};
}
}

#endif
