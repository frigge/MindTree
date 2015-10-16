#include "compositor_plane.h"

template<>
const std::string
PixelPlane::ShaderFiles<Compositor>::
fragmentShader = "../plugins/render/defaultShaders/fullscreenquad.frag";


void CompositorPlane::draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program)
{
    program->setTexture("texture1", _texture01);
    program->setTexture("texture2", _texture02);

    program->setUniform("mixValue", 
    PixelPlane::draw(camera, config, program);
}
