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
enum class GLCullMode : uint8_t { NoCull, Back, Front };
// clang-format on

class GLRenderParameters {

public:
    GLRenderParameters() = default;
    GLBlend blendMode = GLBlend::Disabled;
    GLDepth depthMode = GLDepth::Disabled;
    GLCullMode cullMode = GLCullMode::NoCull;
    GLPolyMode polyMode = GLPolyMode::Fill;

    void apply();
};

class GLRenderManager {
public:
    static void draw(GLFrameBufferBase* frameBuffer,
        GLShader* shader,
        CameraBase* camera,
        VisualObjectBase* visualObject,
        GLRenderParameters params = GLRenderParameters())
    {
        preDraw(frameBuffer, shader, camera, params);

        assert(visualObject);
        drawInternal(shader, *visualObject);
    }

    static void draw(GLFrameBufferBase* frameBuffer,
        GLShader* shader,
        CameraBase* camera,
        std::vector<VisualObjectSprite2D>& visualObjects,
        GLRenderParameters params = GLRenderParameters())
    {
        preDraw(frameBuffer, shader, camera, params);

        for (auto& vo : visualObjects)
            drawInternal(shader, vo);
    }

private:
    static void drawInternal(GLShader* shader,
        VisualObjectBase& visualObject);

    static void preDraw(GLFrameBufferBase* frameBuffer, GLShader* shader,
        CameraBase* camera, GLRenderParameters params);
};

#endif // GL_RENDERMANAGER_H
