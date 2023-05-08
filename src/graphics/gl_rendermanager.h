#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "camera.h"
#include "entity.h"
#include "gl_framebuffer.h"

// clang-format off
enum class GLBlend : uint8_t { Disabled, OneMinusAlpha };
enum class GLDepth : uint8_t { Disabled, Enabled };
enum class GLPolyMode : uint8_t { Fill, Lines };
// clang-format on

class GLRenderParameters {
    static GLRenderParameters s_currentParams;

public:
    GLRenderParameters() = default;
    GLBlend blendMode = GLBlend::Disabled;
    GLDepth depthMode = GLDepth::Disabled;
    GLPolyMode polyMode = GLPolyMode::Fill;

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
