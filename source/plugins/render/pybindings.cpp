#include "boost/python.hpp"
#include "render.h"

namespace bpy = boost::python;
using namespace MindTree::GL;

BOOST_PYTHON_MODULE(pyrender)
{
    bpy::class_<RenderTree>("RenderTree");
    bpy::class_<RenderPass>("RenderPass");
    bpy::class_<RenderConfig>("RenderConfig");
}
