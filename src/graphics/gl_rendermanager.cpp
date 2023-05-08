#include "gl_rendermanager.h"
#include "gl_shader.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <cassert>
#include <unordered_map>

GLRenderParameters GLRenderParameters::s_currentParams;
namespace {
void generateMipMap(GLFrameBufferBase* fb)
{
    if (GLFrameBuffer* fbTexture = dynamic_cast<GLFrameBuffer*>(fb)) {
        fbTexture->generateMipMap();
    }
}
} // namespace

void GLRenderManager::draw(GLShader* shader,
    GLFrameBufferBase* frameBuffer,
    CameraBase* camera,
    EntityBase* drawable,
    bool clear,
    GLRenderParameters params)
{
    frameBuffer->bind();

    if (clear) {
        bool withDepth = (params.depthMode == GLDepth::Enabled);
        frameBuffer->clear(withDepth);
    }

    shader->bind();
    shader->resetVariables();

    if (camera)
        camera->updateUniforms(shader);

    params.apply();

    drawable->applyUniformsAndDraw(shader);

    generateMipMap(frameBuffer);
}

void GLRenderParameters::apply()
{
    if (s_currentParams.blendMode != blendMode) {
        s_currentParams.blendMode = blendMode;
        switch (blendMode) {
        case GLBlend::Disabled: {
            glDisable(GL_BLEND);
        } break;
        case GLBlend::OneMinusAlpha: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } break;
        default:
            LOGE("Not supported blend mode: " << (int)blendMode);
            assert(false);
            break;
        }
    }

    if (s_currentParams.depthMode != depthMode) {
        s_currentParams.depthMode = depthMode;
        switch (depthMode) {
        case GLDepth ::Disabled: {
            glDisable(GL_DEPTH_TEST);
        } break;
        case GLDepth ::Enabled: {
            glEnable(GL_DEPTH_TEST);
        } break;
        default:
            LOGE("Not supported depth mode: " << (int)depthMode);
            assert(false);
            break;
        }
    }

    if (s_currentParams.polyMode != polyMode) {
        s_currentParams.polyMode = polyMode;
        switch (polyMode) {
        case GLPolyMode::Fill: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } break;
        case GLPolyMode::Lines: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } break;

        default:
            LOGE("Not supported poly mode: " << (int)polyMode);
            assert(false);
            break;
        }
    }
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
}
