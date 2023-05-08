#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/entity.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"
#include "graphics/model3d.h"

#include <SDL2/SDL.h>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
static fs::path resourceDir(RESOURCE_DIR);

class OpenGLApp3D : public Application {
    EntityMesh3D m_mesh3d;
    EntitySprite2D m_spriteReceive3D, m_spriteAccumulator;
    GLFrameBuffer m_frameBufferReceive3D, m_frameBufferAccumulator;

    std::shared_ptr<GLShader> m_shaderDefault2D, m_shaderDefault3D;
    CameraPerspective m_camera;
    int m_dirtyLevel = 0;

public:
    void resetDirty() { m_dirtyLevel = 0; }
    void init() override
    {
        m_camera.setAR((float)m_window.getSize().x / m_window.getSize().y);

        m_window.setMouseScrollEvent( // zoom on scroll
            [this](float diff, glm::ivec2 mousePos) {
                float scaleFactor = pow(1.1f, -diff);
                float distance = m_camera.getDistance();

                distance *= scaleFactor;
                distance = glm::clamp(distance, 0.1f, 1000.f);
                m_camera.setDistanceFromAim(distance);
                resetDirty();
            });

        m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos,
                glm::ivec2 delta, DragState dragState) {
                const float offsetScale = 2.f / m_window.getSize().x;
                m_camera.pan(glm::vec2(delta) * offsetScale);
                resetDirty();
            });

        m_window.setMouseDragEvent(MouseButton::Left, // rotate on LMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos,
                glm::ivec2 delta, DragState dragState) {
                const float offsetScale = M_PI * 2.f / m_window.getSize().x;
                m_camera.rotateAroundAim(glm::vec2(delta) * offsetScale);
                resetDirty();
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_camera.setAR((float)newSize.x / newSize.y);
                resetDirty();
            });

        m_shaderDefault2D = GLShaderManager::get().getDefaultShader2d();
        m_shaderDefault3D = GLShaderManager::get().getDefaultShader3d();

        glm::vec2 frameBufferSize(1000, 500);
        m_frameBufferReceive3D.create(frameBufferSize);
        m_frameBufferAccumulator.create(frameBufferSize, GLTexture2D::Format::RGB_32F);

        m_frameBufferReceive3D.setClearColor({ .08f, .09f, .1f, 1.f });

        m_camera.setJitterAA(1.f / frameBufferSize);

        m_spriteReceive3D.setUniform("texture0", m_frameBufferReceive3D.getTexture());
        m_spriteAccumulator.setUniform("texture0", m_frameBufferAccumulator.getTexture());

        auto textureAO = std::make_shared<GLTexture2D>(Image(resourceDir / "models3d" / "AO.png"));
        auto models = MeshReader::read(resourceDir / "models3d" / "stanford_dragon.obj");
        assert(models.size());
        auto glMesh3d = std::make_shared<GLMeshTriIndices>(models[0]);
        m_mesh3d.setMesh(glMesh3d);
        m_mesh3d.setUniform("texture0", textureAO);
    }

    void updateWindow(float dt) override
    {
        // render 3d to framebuffer 3d
        auto startFrameTime = SDL_GetPerformanceCounter();
        float freq = SDL_GetPerformanceFrequency();
        while (true) {
            m_camera.setJitterEnabled(m_dirtyLevel != 0);
            m_spriteReceive3D.setUniform("opacity", 1.f / (m_dirtyLevel + 1));

            GLRenderParameters params3d { GLBlend::Disabled, GLDepth::Enabled, GLPolyMode::Lines };
            GLRenderManager::draw(m_shaderDefault3D.get(), &m_frameBufferReceive3D, &m_camera, &m_mesh3d, true, params3d);

            // render framebuffer to accumulator with alpha
            GLRenderParameters paramsAccumulate { GLBlend::OneMinusAlpha, GLDepth::Disabled };
            GLRenderManager::draw(m_shaderDefault2D.get(), &m_frameBufferAccumulator, nullptr, &m_spriteReceive3D, false, paramsAccumulate);
            m_dirtyLevel++;

            float frameTime = (float)((SDL_GetPerformanceCounter() - startFrameTime) / freq);

            if (m_dirtyLevel > 4000 || frameTime > 0.01f)
                break;
        }

        // render accumulator to screen
        GLRenderParameters paramsPresent { GLBlend::Disabled, GLDepth::Disabled };
        GLRenderManager::draw(m_shaderDefault2D.get(), &m_window, nullptr, &m_spriteAccumulator, false, paramsPresent);
    }
};
typedef OpenGLApp3D App;
#endif // OPENGLAPP2D_H
