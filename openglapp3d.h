#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/entity.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"
#include "graphics/model3d.h"
#include "graphics/visualobject.h"

#include <SDL2/SDL.h>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
static fs::path resourceDir(RESOURCE_DIR);

class OpenGLApp3D : public Application {
    VisualObject3D m_mesh3d;
    VisualObjectSprite2D m_spriteReceive3D, m_spriteAccumulator;
    GLFrameBufferDepth m_frameBufferReceive3D;
    GLFrameBuffer m_frameBufferAccumulator;

    GLShader m_shaderDefault2D, m_shaderDefault3D;
    CameraPerspectiveJittered m_camera;
    int m_dirtyLevel = 0;

public:
    void resetDirty() { m_dirtyLevel = 0; }
    void init() override
    {
        m_camera.setAR((float)m_window.getSize().x / m_window.getSize().y);

        m_window.setScreenResizeEvent(
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_frameBufferReceive3D.resize(newSize);
                m_frameBufferAccumulator.resize(newSize);
                m_camera.setFramebufferSize(newSize);
                resetDirty();
            });

        m_window.setMouseScrollEvent( // zoom on scroll
            [this](float diff, glm::ivec2 mousePixelPos) {
                float scaleFactor = pow(1.1f, -diff);
                float distance = m_camera.getDistance();
                glm::vec2 mousePos = m_window.toNormalizedPos(mousePixelPos, false);

                distance *= scaleFactor;
                m_camera.pan(mousePos * (scaleFactor - 1.f));
                m_camera.setDistanceFromAim(glm::clamp(distance, 0.1f, 1000.f));
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

        m_shaderDefault2D = GLShader::FromFile("default2d.vert", "default2d.frag");
        m_shaderDefault3D = GLShader::FromFile("default3d.vert", "default3d.frag");

        m_frameBufferAccumulator.create(m_window.getSize(), TexelFormat::RGB_32F);
        m_frameBufferReceive3D.create(m_window.getSize(), TexelFormat::RGB_8);
        m_frameBufferReceive3D.setClearColor({ .08f, .09f, .1f, 1.f });
        m_camera.setFramebufferSize(m_window.getSize());

        m_camera.setJitterDOF(0.1f);

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
        while (m_dirtyLevel <= 4000) {
            m_camera.setJitterEnabled(m_dirtyLevel != 0); // zero frame without jitter

            GLRenderParameters params3d { GLBlend::Disabled, GLDepth::Enabled, GLCullMode::NoCull, GLPolyMode::Lines };
            m_frameBufferReceive3D.clear();
            GLRenderManager::draw(&m_frameBufferReceive3D, &m_shaderDefault3D, &m_camera, &m_mesh3d, params3d);

            // render framebuffer to accumulator with alpha
            m_spriteReceive3D.setUniform("opacity", 1.f / (m_dirtyLevel + 1));
            GLRenderParameters paramsAccumulate { GLBlend::OneMinusAlpha, GLDepth::Disabled };
            GLRenderManager::draw(&m_frameBufferAccumulator, &m_shaderDefault2D, nullptr, &m_spriteReceive3D, paramsAccumulate);
            m_dirtyLevel++;

            float frameTime = (float)((SDL_GetPerformanceCounter() - startFrameTime) / freq);

            if (frameTime > 0.001f)
                break;

            m_dirtyLevel++;
        }

        // render accumulator to screen
        GLRenderParameters paramsPresent { GLBlend::Disabled, GLDepth::Disabled };
        GLRenderManager::draw(&m_window, &m_shaderDefault2D, nullptr, &m_spriteAccumulator, paramsPresent);
    }
};
typedef OpenGLApp3D App;
#endif // OPENGLAPP2D_H
