#include "gl_rendermanager.h"
#include "gl_shader.h"

#include <SDL2/SDL.h>

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

void GLRenderManager::draw(
    GLShader& shader,
    GLFrameBufferBase& frameBuffer,
    CameraBase* camera,
    const std::vector<std::unique_ptr<EntityBase>>& allContext)
{
    frameBuffer.bind();

    shader.bind();
    shader.resetVariables(); // clear shader stuff before applying uniforms

    if (camera)
        camera->updateUniforms(&shader);

    for (const auto& obj : allContext)
        obj->applyUniformsAndDraw(&shader);

    generateMipMap(&frameBuffer);
}

void GLRenderManager::draw(
    GLShader& shader,
    GLFrameBufferBase& frameBuffer,
    CameraBase* camera,
    EntityBase* drawable)
{
    frameBuffer.bind();

    shader.bind();
    shader.resetVariables();

    if (camera)
        camera->updateUniforms(&shader);

    drawable->applyUniformsAndDraw(&shader);

    generateMipMap(&frameBuffer);
}
