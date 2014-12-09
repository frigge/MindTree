#ifndef EMPTY_RENDERER_H_OV3SYN5E
#define EMPTY_RENDERER_H_OV3SYN5E

#include "primitive_renderer.h"

class Empty;
namespace MindTree
{

namespace GL
{
class EmptyRenderer : public ShapeRendererGroup
{
public:
    EmptyRenderer (const EmptyPtr e);
    virtual ~EmptyRenderer ();

private:
    const std::shared_ptr<Empty> _empty;
};
} /* GL */  
} /* MindTree */ 

#endif /* end of include guard: EMPTY_RENDERER_H_OV3SYN5E */
