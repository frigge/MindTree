#ifndef MT_GL_AMBIENT_OCCLUSION_H
#define MT_GL_AMBIENT_OCCLUSION_H

#include "render_block.h"

namespace MindTree {
namespace GL {

class AmbientOcclusionPlane : public PixelPlane
{
public:
    AmbientOcclusionPlane();

    void setSamples(int samples);
    void setSearchRadius(float radius);
    void setAmbientColor(glm::vec4 color);

protected:
    virtual void init(std::shared_ptr<ShaderProgram> prog);
    virtual void draw(const CameraPtr camera, const RenderConfig &config, std::shared_ptr<ShaderProgram> program);

private:
    void initSamplingTexture();

    std::shared_ptr<Texture> _samplingPattern;
    std::atomic<double> _searchRadius;
    std::atomic<int> _numSamples;
    glm::vec4 _ambientColor;

    std::atomic<bool> _samplesChanged;
};

class AmbientOcclusionBlock : public RenderBlock
{
public:
    AmbientOcclusionBlock();

    virtual void init();
    void setGeometry(std::shared_ptr<Group> grp);

private:
    AmbientOcclusionPlane *_aoplane;

};

}
};
#endif
