#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/entity.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"
#include "graphics/model3d.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
static fs::path resourceDir(RESOURCE_DIR);

class OpenGLApp3D : public Application {
    EntityMesh3D m_mesh3d;
    std::shared_ptr<GLShader> m_shaderDefault3D;
    CameraPerspective m_camera;

public:
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
            });

        m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos,
                glm::ivec2 delta, DragState dragState) {
                const float offsetScale = 2.f / m_window.getSize().x;
                m_camera.pan(glm::vec2(delta) * offsetScale);
                // camera pan
            });

        m_window.setMouseDragEvent(MouseButton::Left, // rotate on LMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos,
                glm::ivec2 delta, DragState dragState) {
                const float offsetScale = M_PI * 2.f / m_window.getSize().x;
                m_camera.rotateAroundAim(glm::vec2(delta) * offsetScale);
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_camera.setAR((float)newSize.x / newSize.y);
            });

        // Application::init();
        m_shaderDefault3D = GLShaderManager::get().getDefaultShader3d();

        auto textureAO = std::make_shared<GLTexture2D>(Image(resourceDir / "models3d" / "AO.png"));

        auto models = MeshReader::read(resourceDir / "models3d" / "stanford_dragon.obj");
        assert(models.size());

        auto glMesh3d = std::make_shared<GLMeshTriIndices>(models[0]);
        m_mesh3d.setMesh(glMesh3d);
        m_mesh3d.setUniform("texture0", textureAO);

        m_window.enableDepthTest(true);
    }

    void updateWindow(float dt) override
    {
        GLRenderManager rm;

        m_window.clear(0.16f, 0.16f, 0.16f, 1);
        rm.draw(*m_shaderDefault3D, m_window, &m_camera, &m_mesh3d);
    }
};
typedef OpenGLApp3D App;
#endif // OPENGLAPP2D_H
