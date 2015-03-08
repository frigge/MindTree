#include "rsm_computation_plane.h"

using namespace MindTree;
using namespace GL;

std::weak_ptr<ShaderProgram> RSMAccumulationPlane::_defaultProgram;

std::shared_ptr<ShaderProgram> RSMAccumulationPlane::getProgram()
{
    std::shared_ptr<ShaderProgram> prog;
    if(_defaultProgram.expired()) {
        prog = std::make_shared<ShaderProgram>();

        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/fullscreenquad.vert", 
                                ShaderProgram::VERTEX);
        prog
            ->addShaderFromFile("../plugins/render/defaultShaders/rsm_indirect_lighting.frag", 
                                ShaderProgram::FRAGMENT);
        _defaultProgram = prog;
    }

    return _defaultProgram.lock();
}

