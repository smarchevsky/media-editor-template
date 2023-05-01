#include "gl_rendermanager.h"
#include "gl_shader.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <cassert>
#include <unordered_map>

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
        bool withDepth = (params.depth == GLDepth::Enabled);
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
    switch (blend) {
    case GLBlend::Disabled: {
        glDisable(GL_BLEND);
    } break;
    case GLBlend::OneMinusAlpha: {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } break;
    default:
        LOGE("Not supported blend: " << (int)blend);
        assert(false);
        break;
    }

    switch (depth) {
    case GLDepth ::Disabled: {
        glDisable(GL_DEPTH_TEST);
    } break;
    case GLDepth ::Enabled: {
        glEnable(GL_DEPTH_TEST);
    } break;
    default:
        break;
    }
}
