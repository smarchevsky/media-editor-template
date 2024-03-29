#ifndef OPENGLAPP2D_H
#define OPENGLAPP2D_H

#include "application.h"

#include "graphics/gl_shader.h"
#include "graphics/visualobject.h"

#include "graphics/gl_rendermanager.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

class OpenGLApp2D : public Application {
    std::vector<VisualObjectSprite2D> m_sprites;
    GLShader m_shaderDefault2d;
    GLFrameBuffer m_fb;
    CameraOrtho m_cameraView;
    CameraRect m_cameraViewFramebuffer;

public:
    void init() override
    {
        m_cameraView.setViewportSize(m_window.getSize());

        ///////////////////////////////////////////////////

        addFileInteractionInfo("Primary", "png,jpg", nullptr, nullptr);

        // open file
        m_window.addKeyDownEvent(SDLK_o, KMOD_CTRL,
            std::bind(&Application::openFileDialog, this, "Primary"));

        // save file
        m_window.addKeyDownEvent(SDLK_s, KMOD_CTRL,
            std::bind(&Application::saveFileOptionalDialog, this, "Primary", false));

        // save file as
        m_window.addKeyDownEvent(SDLK_s, KMOD_CTRL | KMOD_SHIFT,
            std::bind(&Application::saveFileOptionalDialog, this, "Primary", true));

        ///////////////////////////////////////////////////
        m_window.setMouseScrollEvent( // zoom on scroll
            [this](float diff, glm::ivec2 mousePos) {
                float scaleFactor = pow(1.1f, -diff);
                m_cameraView.multiplyScaleOffseted(scaleFactor, mousePos);
            });

        m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_cameraView.addOffset_View(glm::vec2(-currentDelta));
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_cameraView.setViewportSize(glm::vec2(newSize));
            });
        m_window.setClearColor({ .13f, .14f, .15f, 1.f });

        m_shaderDefault2d = GLShader::FromFile("default2d.vert", "default2d.frag");

        m_fb.create({ 2048, 2048 }, TexelFormat::RGBA_8);
        m_fb.getTexture()->setFiltering(GLTexture2D::Filtering::LinearMipmap);
        m_fb.getTexture()->setWrapping(GLTexture2D::Wrapping::ClampEdge);
        m_fb.setClearColor({ .23f, .24f, .25f, 1.f });

        auto texChecker = std::make_shared<GLTexture2D>(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
        // auto texLiza = std::make_shared<GLTexture2D>(Image(projectDir / "resources" / "mona_liza.jpg"));
        //   m_textureDefault.fromImage(Image({ 128, 128 }, glm::ivec4(100, 200, 255, 255)));
        {
            VisualObjectSprite2D s; // CHECKER
            // s.setPos({ 0.5f, 0.f });
            s.setUniform("texture0", texChecker);
            m_sprites.push_back(s);
        }
        {
            VisualObjectSprite2D s; // FRAME BUFFER
            s.setUniform("texture0", m_fb.getTexture());
            s.setPos({ -1.5f, -0.3f });
            m_sprites.push_back(s);
        }
        m_cameraViewFramebuffer.setViewRect({ -2, -2, 2, 2 });
    }

    void updateWindow(float dt) override
    {

        m_sprites[0].addRotation(dt * 0.1f);

        m_fb.clear();
        GLRenderManager::draw(&m_fb, &m_shaderDefault2d, &m_cameraViewFramebuffer, &m_sprites[0]);

        m_window.clear();
        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, m_sprites);
    }
};
typedef OpenGLApp2D App;
#endif // OPENGLAPP2D_H
