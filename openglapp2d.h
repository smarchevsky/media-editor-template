#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/drawable.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

class OpenGLApp2D : public Application {
    std::vector<Sprite2D> m_sprites;
    std::shared_ptr<GLShader> m_shaderDefault2d;
    GLFrameBuffer m_fb;
    CameraOrtho m_camera;

public:
    void init() override
    {
        m_camera.setViewportSize(m_window.getSize());

        m_window.setMouseScrollEvent( // zoom on scroll
            [this](float diff, glm::ivec2 mousePos) {
                float scaleFactor = pow(1.1f, -diff);
                m_camera.multiplyScaleOffseted(scaleFactor, mousePos);
            });

        m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_camera.addOffset_View(glm::vec2(-currentDelta));
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_camera.setViewportSize(glm::vec2(newSize));
            });

        // Application::init();
        m_shaderDefault2d = GLShaderManager::get().getDefaultShader2d();

        m_fb.create({ 2048, 2048 });
        m_fb.getTexture()->setFiltering(GLTexture::Filtering::LinearMipmap);
        m_fb.getTexture()->setWrapping(GLTexture::Wrapping::ClampEdge);

        auto texChecker = SHARED_TEXTURE(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
        auto texLiza = SHARED_TEXTURE(Image(projectDir / "resources" / "mona_liza.jpg"));
        //  m_textureDefault.fromImage(Image({ 128, 128 }, glm::ivec4(100, 200, 255, 255)));
        {
            Sprite2D s; // 0
            s.setPos({ 0.5f, 0.f });
            s.setUniform("texture0", texChecker);
            s.setUniform("texture1", texLiza);
            m_sprites.push_back(s);
        }
        {
            Sprite2D s; // 1
            s.setPos({ -0.5f, -0.3f });
            s.setUniform("texture0", texChecker);
            s.setUniform("texture1", m_fb.getTexture());
            m_sprites.push_back(s);
        }
    }

    void updateWindow(float dt) override
    {
        GLRenderManager rm;
        m_sprites[0].addRotation(dt * 0.1f);

        m_fb.clear(0, 0, 0, 1);
        rm.draw(*m_shaderDefault2d, m_fb, nullptr, &m_sprites[0]);

        m_window.clear(0.16f, 0.16f, 0.16f, 1);
        rm.draw(*m_shaderDefault2d, m_window, &m_camera, &m_sprites[1]);
    }
};
typedef OpenGLApp2D App;
#endif // OPENGLAPP2D_H
