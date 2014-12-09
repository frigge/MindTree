#include "../datatypes/Object/object.h"

#include "camera_renderer.h"

using namespace MindTree;
using namespace MindTree::GL;

CameraRenderer::CameraRenderer(const CameraPtr c)
    : _camera(c)
{
}

CameraRenderer::~CameraRenderer()
{
}
