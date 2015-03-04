#include "../datatypes/Object/lights.h"
#include "glm/gtc/matrix_transform.hpp"

#include "light_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

PointLightRenderer::PointLightRenderer(const PointLightPtr l)
    : _light(l)
{
    auto *circle1 = new CircleRenderer(this);
    auto *circle2 = new CircleRenderer(this);
    auto *circle3 = new CircleRenderer(this);
    circle1->setStaticTransformation(glm::scale(glm::mat4(), glm::vec3(.3)));

    circle2->setStaticTransformation(glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0))
                                       * glm::scale(glm::mat4(), glm::vec3(.3)));

    circle3->setStaticTransformation(glm::rotate(glm::mat4(), 90.f, glm::vec3(0, 0, 1))
                                       * glm::scale(glm::mat4(), glm::vec3(.3)));

    setBorderColor(glm::vec4(1, 1, 0, 1));
    setBorderWidth(2);

    setFixedScreenSize(true);
    setTransformation(l->getWorldTransformation());
}

SpotLightRenderer::SpotLightRenderer(const SpotLightPtr l)
    : _light(l)
{
    auto *cone = new ConeRenderer(this);
    auto *circle1 = new CircleRenderer(this);
    cone->setStaticTransformation(glm::scale(glm::mat4(), glm::vec3(.5, 1, .5)));
    circle1->setStaticTransformation(glm::translate(glm::mat4(), glm::vec3(0, -1, 0)));
    setStaticTransformation(glm::rotate(glm::mat4(), 90.f, glm::vec3(1, 0, 0)));

    setTransformation(l->getWorldTransformation());

    setFillColor(glm::vec4(1, 1, 0, .5));
    setBorderColor(glm::vec4(1, 1, 0, 1));
    setBorderWidth(2);
    setFixedScreenSize(true);
}

DistantLightRenderer::DistantLightRenderer(const DistantLightPtr l)
    : _light(l)
{
}
