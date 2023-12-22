#include "gl_rendermanager.h"
#include "gl_shader.h"

#include "gl_framebuffer.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <cassert>
#include <unordered_map>

static GLRenderParameters s_currentParams;

void GLRenderManager::preDraw(GLFrameBufferBase* frameBuffer, GLShader* shader, CameraBase* camera, GLRenderParameters params)
{
    assert(shader);
    assert(frameBuffer);

    frameBuffer->bind();

    // reset all uniforms to default values
    shader->bindAndResetUniforms();

    // set camera data, if no camera - there will be default uniforms
    if (camera)
        shader->setCameraUniforms(camera->updateAndGetUniforms());

    params.apply();
}

void GLRenderManager::drawInternal(GLShader* shader, VisualObjectBase& visualObject)
{
    if (visualObject.getMesh()) {
        shader->setUniforms(visualObject.updateAndGetUniforms());
        visualObject.getMesh()->draw();
    } else {
        LOGE("No mesh :(");
    }
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
        case GLBlend::Additive: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        } break;
        default:
            LOGE("Not supported blend mode: " << (int)blendMode);
            assert(false);
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
        }
    }

    if (s_currentParams.cullMode != cullMode) {
        s_currentParams.cullMode = cullMode;

        switch (cullMode) {
        case GLCullMode::NoCull: {
            glDisable(GL_CULL_FACE);
        } break;

        case GLCullMode::Back: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } break;

        case GLCullMode::Front: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } break;

        default:
            LOGE("Not supported cull mode: " << (int)cullMode);
            assert(false);
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
        }
    }
}
