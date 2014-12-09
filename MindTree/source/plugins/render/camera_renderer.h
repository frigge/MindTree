#ifndef CAMERA_RENDERER_H_Q2AK6MQA
#define CAMERA_RENDERER_H_Q2AK6MQA

#include "memory"
#include "primitive_renderer.h"

class Camera;
namespace MindTree
{

namespace GL
{

class CameraRenderer : public ShapeRendererGroup
{
public:
    CameraRenderer (const CameraPtr c);
    virtual ~CameraRenderer ();

private:
    const std::shared_ptr<Camera> _camera;
};
    
} /* GL */ 
} /* MindTree */ 

#endif /* end of include guard: CAMERA_RENDERER_H_Q2AK6MQA */
