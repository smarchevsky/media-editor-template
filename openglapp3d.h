#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/drawable.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"
#include "graphics/model3d.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
static fs::path resourceDir(RESOURCE_DIR);

class OpenGLApp3D : public Application {
    Mesh3D m_mesh3d;
    std::shared_ptr<GLShader> m_shaderDefault3D;
    CameraPerspective m_camera;
    glm::vec2 m_sceneRot;

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
                m_camera.setDistance(distance);
            });

        m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos,
                glm::ivec2 delta, DragState dragState) {
                constexpr float offsetScale = 0.001f;

                glm::mat4 invView = glm::inverse(m_camera.getView());
                auto aim = m_camera.getAim();
                auto pos = m_camera.getPos();

                float distance = glm::distance(pos, aim);
                glm::vec3 right = invView[0];
                glm::vec3 up = invView[1];
                auto offset = (-right * (float)delta.x + up * (float)delta.y)
                    * offsetScale * distance;

                m_camera.setPos(pos + offset);
                m_camera.setAim(aim + offset);
            });

        m_window.setMouseDragEvent(MouseButton::Left, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos,
                glm::ivec2 delta, DragState dragState) {
                constexpr float offsetScale = 0.003f;

                m_sceneRot += glm::vec2(-delta.x, delta.y) * offsetScale;
                m_sceneRot.y = glm::clamp(m_sceneRot.y, -(float)M_PI_2 + 0.001f, (float)M_PI_2 - 0.001f);
                m_sceneRot.x = fmodf(m_sceneRot.x, M_PI * 2);

                auto origin = m_camera.getAim();
                auto rotatedVector1 = origin + glm::rotateZ(glm::rotateX(glm::vec3(0.f, m_camera.getDistance(), 0.f), m_sceneRot.y), m_sceneRot.x);
                m_camera.setPos(rotatedVector1);
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_camera.setAR((float)newSize.x / newSize.y);
            });

        // Application::init();
        m_shaderDefault3D = GLShaderManager::get().getDefaultShader3d();

        auto textureAO = SHARED_TEXTURE(
            Image(resourceDir / "models3d" / "AO.png"));

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
