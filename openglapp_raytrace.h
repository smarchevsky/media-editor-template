#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/entity.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"
#include "graphics/model3d.h"
#include "raytracing/rt_texture_assembler.h"
#include <glm/gtx/rotate_vector.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
static fs::path resourceDir(RESOURCE_DIR);

class OpenGLApp3D : public Application {
    EntityMesh3D m_cubeMesh;

    GLShader m_shaderDefault3D;
    CameraPerspective m_camera;
    double m_time = 0;
    glm::vec3 m_boxScale = glm::vec3(3);

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

        m_shaderDefault3D = GLShader::FromFile("default3d.vert", "raytracing16bit.frag");

        auto cubeModel = MeshReader::read(resourceDir / "models3d/cube.obj");
        assert(cubeModel.size());
        auto glMesh3d = std::make_shared<GLMeshTriIndices>(cubeModel[0]);

        auto dragonModel = MeshReader::read(resourceDir / "models3d/stanford_dragon.obj");
        assert(dragonModel.size());

        BVH::BVHBuilder bvh;
        auto RTTexture = std::make_shared<GLTexture2D>(
            RTTextureAssembler::assemble(dragonModel[0], bvh));
        auto bMin = bvh.getNodes()[0].aabb.getMin();
        auto bMax = bvh.getNodes()[0].aabb.getMax();
        m_boxScale = bMax - bMin;

        m_cubeMesh.setMesh(glMesh3d);
        m_cubeMesh.initializeUniform("texGeometry", RTTexture);
        m_cubeMesh.initializeUniform("texGeometrySize", RTTexture->getSize());
        m_window.setClearColor({ 0.1f, 0.1f, 0.2f, 1.f });
    }

    void updateWindow(float dt) override
    {
        m_time += dt;
        glm::mat4 m = glm::mat4(1);

        m = glm::rotate(m, (float)m_time * 0.1f, glm::normalize(glm::vec3(1, 1, 1)));
        m = glm::translate(m, glm::vec3(0, 0, 0.1f * sinf(m_time * 2)));
        m = glm::scale(m, m_boxScale);

        m_cubeMesh.setTransform(m);
        GLRenderParameters params3d { GLBlend::Disabled, GLDepth::Enabled };
        GLRenderManager::draw(&m_shaderDefault3D, &m_window, &m_camera, &m_cubeMesh, true, params3d);
    }
};
typedef OpenGLApp3D App;
#endif // OPENGLAPP2D_H
