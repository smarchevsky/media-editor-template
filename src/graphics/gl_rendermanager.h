#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "camera.h"
#include "entity.h"
#include "gl_framebuffer.h"

struct GLRenderParameters {
    GLRenderParameters() = default;

    // clang-format off
    enum class Blend : uint8_t { Disabled, OneMinusAlpha }
        blend = Blend::Disabled;

    enum class Depth : uint8_t { Disabled, Enabled }
        depth = Depth::Disabled;
    // clang-format on

    void apply();
};

class GLRenderManager {
public:
    static void draw(GLShader* shader,
        GLFrameBufferBase* frameBuffer,
        CameraBase* camera,
        EntityBase* drawable,
        bool clear,
        GLRenderParameters params = GLRenderParameters());
};

#endif // GL_RENDERMANAGER_H
