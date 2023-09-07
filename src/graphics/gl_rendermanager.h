#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "camera.h"
#include "visualobject.h"

class GLFrameBufferBase;
class VisualObjectBase;

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
    static void draw(GLFrameBufferBase* frameBuffer,
        GLShader* shader,
        CameraBase* camera,
        VisualObjectBase* visualObject,
        bool clear,
        GLRenderParameters params = GLRenderParameters())
    {
        assert(visualObject);
        preDraw(frameBuffer, shader, camera, params, clear);
        drawInternal(shader, *visualObject);
        postDraw(frameBuffer, shader, camera, params);
    }

    static void draw(GLFrameBufferBase* frameBuffer,
        GLShader* shader,
        CameraBase* camera,
        std::vector<VisualObjectSprite2D>& visualObjects,
        bool clear,
        GLRenderParameters params = GLRenderParameters())
    {
        preDraw(frameBuffer, shader, camera, params, clear);
        for (auto& vo : visualObjects)
            drawInternal(shader, vo);
        postDraw(frameBuffer, shader, camera, params);
    }

private:
    static void drawInternal(
        GLShader* shader,
        VisualObjectBase& visualObject);

    static void preDraw(GLFrameBufferBase* frameBuffer, GLShader* shader,
        CameraBase* camera, GLRenderParameters params, bool clear);

    static void postDraw(GLFrameBufferBase* frameBuffer, GLShader* shader,
        CameraBase* camera, GLRenderParameters params);
};

#endif // GL_RENDERMANAGER_H
