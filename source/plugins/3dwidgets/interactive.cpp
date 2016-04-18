#include "interactive.h"

using namespace MindTree;
using namespace MindTree::GL;
using namespace MindTree::Interactive;

ShapeGroup::ShapeGroup() :
    screenOriented_(false),
    screenSize_(true)
{
}

glm::mat4 ShapeGroup::computeTransformation(const std::shared_ptr<Camera> &cam,
                                          glm::ivec2 pixel,
                                          glm::ivec2 viewportSize) const
{
    auto model = renderer_->getGlobalTransformation();
    auto view = cam->getViewMatrix();
    auto projection = cam->getProjection();
    auto viewProj = projection * view;

    //extract translation from model
    glm::mat4 translate = model * glm::inverse(glm::mat4(glm::mat3(model)));

    auto project = [&] (glm::vec4 pos) -> glm::vec2 {
        glm::vec4 posndc = viewProj * translate * pos;
        posndc /= posndc.w;
        glm::vec2 pospixel = posndc.xy() + glm::vec2(1);
        pospixel /= 2;
        pospixel *= viewportSize;
        return pospixel;
    };

    //compute Screen Orientation
    glm::mat4 camrotation = glm::mat4(glm::mat3(view));
    glm::mat4 invcamrot = glm::inverse(camrotation);

    glm::mat4 finalTransform = translate;

    //compute scale matrix
    if(screenOriented_) {
        finalTransform = translate * invcamrot;
    }

    if (screenSize_) {
        auto scale = glm::mat4(1);

        glm::vec2 x0 = project(invcamrot * glm::vec4(0, 0, 0, 1));
        glm::vec2 x1 = project(invcamrot * glm::vec4(1, 0, 0, 1));

        auto vec = x1 - x0;
        float screenlength = glm::length(vec);

        scale[0].x = 50 / screenlength;
        scale[1].y = 50 / screenlength;
        scale[2].z = 50 / screenlength;

        finalTransform = finalTransform * scale;
        finalTransform[3][3] = 1;
    }

    return finalTransform;
}

bool ShapeGroup::intersect(const std::shared_ptr<Camera> &cam,
                           glm::ivec2 pixel,
                           glm::ivec2 viewportSize,
                           glm::vec3 *hitpoint) const
{
    glm::vec3 hit;
    auto campos = cam->getPosition();
    float depth = std::numeric_limits<float>::max();
    for (const auto &child : children_) {
        child->intersect(cam, pixel, viewportSize, &hit);
        float dist = glm::distance(hit, campos);
        if (depth > dist) {
            depth = dist;

            if (hitpoint)
                *hitpoint = hit;
        }
    }
    return depth < std::numeric_limits<float>::max();
}
