#ifndef OPENGLAPP_SIMPLEIMAGEEDITOR_H
#define OPENGLAPP_SIMPLEIMAGEEDITOR_H

#include "application.h"

#include "graphics/gl_shader.h"
#include "graphics/visualobject.h"

#include "graphics/gl_rendermanager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

class OpenGLApp2D : public Application {
    VisualObjectSprite2D m_spriteImage, m_spriteBrush;
    GLFrameBuffer m_frameBufferImage;
    GLShader m_shaderDefault2d, m_shaderBrush;

    CameraOrtho m_cameraView;
    CameraRect m_cameraImage;

    const float m_brushRadius = 30;
    const float m_imageHalfSize = 512;

public:
    void init() override
    {
        m_cameraView.setViewportSize(m_window.getSize());
        m_cameraView.setScale(1.f);

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

        m_window.setMouseDragEvent(MouseButton::Left, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                GLRenderManager::draw(&m_frameBufferImage, &m_shaderBrush, &m_cameraImage, &m_spriteBrush);
            });

        m_window.setMouseMoveEvent(MouseButton::Left,
            [this](glm::ivec2 currentScreenPos, glm::ivec2 delta) {
                glm::vec2 normalizedScreenPos = m_window.toNormalizedPos(currentScreenPos, true);
                glm::vec2 spritePosWorld = glm::inverse(m_cameraView.getViewMatrix()) * glm::vec4(normalizedScreenPos, 0.f, 1.f);
                m_spriteBrush.setPos(spritePosWorld);
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_cameraView.setViewportSize(glm::vec2(newSize));
            });

        m_window.setClearColor({ .13f, .14f, .15f, 1.f });

        m_shaderDefault2d = GLShader::FromFile("default2d.vert", "default2d.frag");
        m_shaderBrush = GLShader::FromFile("brush2d.vert", "brush2d.frag");

        m_frameBufferImage.create({ 2048, 2048 }, TexelFormat::RGBA_8);
        m_frameBufferImage.getTexture()->setFiltering(GLTexture2D::Filtering::LinearMipmap);
        m_frameBufferImage.getTexture()->setWrapping(GLTexture2D::Wrapping::ClampEdge);
        m_frameBufferImage.setClearColor({ .23f, .24f, .25f, 1.f });

        glm::vec4 imageRect(-m_imageHalfSize, -m_imageHalfSize, m_imageHalfSize, m_imageHalfSize);
        m_spriteImage.setRectSize(imageRect);
        m_spriteImage.setUniform("texture0", m_frameBufferImage.getTexture());

        m_cameraImage.setViewRect(imageRect);

        m_spriteBrush.setRectSize(glm::vec2(-m_brushRadius), glm::vec2(m_brushRadius));
        m_spriteBrush.setUniform("color", glm::vec4(0, 1, 0, 1));
        auto texLiza = std::make_shared<GLTexture2D>(Image(projectDir / "resources" / "mona_liza.jpg"));
        texLiza->setWrapping(GLTexture2D::Wrapping::ClampEdge);
        m_spriteBrush.setUniform("backgroundTexture", texLiza);

        m_spriteBrush.setUniform("imageViewMatrix", (m_cameraImage.getViewMatrix()));
        // m_spriteBrush.setUniform("imageViewMatrix", glm::inverse(m_spriteImage.getModelMatrix()));
    }

    void updateWindow(float dt) override
    {
        m_window.clear();
        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_spriteImage);
        GLRenderManager::draw(&m_window, &m_shaderBrush, &m_cameraView, &m_spriteBrush);
    }
};
typedef OpenGLApp2D App;
#endif // OPENGLAPP_SIMPLEIMAGEEDITOR_H
