#ifndef MT_GL_WIDGETS_INTERACTIVE_H
#define MT_GL_WIDGETS_INTERACTIVE_H

#define GLM_FORCE_SWIZZLE

#include <memory>
#include "graphics/shapes.h"
#include "data/raytracing/ray.h"
#include "../render/primitive_renderer.h"

class Camera;

namespace MindTree {
namespace Interactive {

class Shape
{
public:
	virtual bool intersect(const Ray &ray, float *depth) const = 0;
};

template<typename T>
	class ShapeT : public Shape
{
public:
	ShapeT(const T shape) : shape_(shape) {}

	bool intersect(const Ray &ray, float *depth) const
	{
		return ray.intersect(shape_, depth);
	}

private:
	T shape_;
};

struct ShapeGroup
{
	ShapeGroup();
	bool intersect(const Ray &ray, float *depth) const;

	template<typename T, typename ... A>
	void addShape(A ... args)
	{
		shapes.push_back(std::make_unique<Interactive::ShapeT<T>>(T(args ...)));
	}

    std::vector<std::unique_ptr<Shape>> shapes;
};
}
}

#endif
