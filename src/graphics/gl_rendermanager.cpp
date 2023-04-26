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
    GLFrameBufferBase* frameBuffer,
    CameraBase* camera,
    const std::vector<std::unique_ptr<DrawableBase>>& allContext)
{
    std::unordered_map<GLShader*, std::vector<DrawableBase*>> shadersObjectMap;

    for (const auto& contextObject : allContext) {
        if (contextObject) {
            if (auto shader = contextObject->getShaderInstance().getShader()) {
                auto& s = shadersObjectMap[shader];
                s.push_back(contextObject.get());
            }
        }
    }

    frameBuffer->bind();
    for (const auto& shadersObjectPair : shadersObjectMap) {
        GLShader* shader = shadersObjectPair.first;
        auto& shaderRelatedContext = shadersObjectPair.second;
        shader->bind();
        shader->resetVariables(UniformDependency::View); // clear view-related stuff before camera apply

        CameraBase::updateShaderUniforms(camera, shader);

        for (auto& drawableObject : shaderRelatedContext) {
            drawableObject->draw(false);
        }
    }

    generateMipMap(frameBuffer);
}

void GLRenderManager::draw(
    GLFrameBufferBase* frameBuffer,
    CameraBase* camera,
    DrawableBase* drawable)
{
    frameBuffer->bind();
    GLShader* shader = drawable->getShaderInstance().getShader();
    shader->bind();
    shader->resetVariables(UniformDependency::View);
    if (camera) {
    }

    CameraBase::updateShaderUniforms(camera, shader);

    drawable->draw(false);

    generateMipMap(frameBuffer);
}
