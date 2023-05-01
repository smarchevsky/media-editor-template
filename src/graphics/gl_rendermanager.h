#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "camera.h"
#include "entity.h"
#include "gl_framebuffer.h"

// clang-format off
enum class GLBlend : uint8_t { Disabled, OneMinusAlpha };
enum class GLDepth : uint8_t { Disabled, Enabled };
// clang-format on

struct GLRenderParameters {
    GLRenderParameters() = default;
    GLBlend blend = GLBlend::Disabled;
    GLDepth depth = GLDepth::Disabled;

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
