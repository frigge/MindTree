#include "interactive.h"

using namespace MindTree;
using namespace MindTree::GL;
using namespace MindTree::Interactive;

ShapeGroup::ShapeGroup()
{
}

bool ShapeGroup::intersect(const Ray &ray, float *depth) const
{
    float mind = std::numeric_limits<float>::max();
    bool hit{false};
    for (const auto &shape : shapes) {
	    float d = mind;
		hit = shape->intersect(ray, &d);
        if (mind > d) {
            mind = d;
        }
    }
	if (*depth > mind) {
		*depth = mind;
		return hit;
	}
	return false;
}
