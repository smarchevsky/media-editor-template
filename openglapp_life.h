#ifndef OPENGLAPP_LIFE_H
#define OPENGLAPP_LIFE_H

#include "application.h"

#include "graphics/gl_shader.h"
#include "graphics/visualobject.h"

#include "graphics/gl_rendermanager.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

static const std::string shaderCodeDefault2d_VS = textFromFile(shaderDir / "default2d.vert");
static const std::string shaderCodeDefault2d_PS = textFromFile(shaderDir / "default2d.frag");

static const std::string shaderCodeConweysLife_PS = R"(
#version 330 core
in vec2 uv;
uniform sampler2D texture0;
out float FragColor;

#define ImageSize textureSize(texture0, 0)
int cell(in ivec2 p) { return (texelFetch(texture0, p % ivec2(ImageSize), 0).x > 0.5) ? 1 : 0; }

void main() {
    // ivec2 px = ivec2(uv * ImageSize);
    ivec2 px = ivec2(gl_FragCoord.xy);
    int k =
          cell(px + ivec2(-1,-1)) + cell(px + ivec2(0,-1)) + cell(px + ivec2(1,-1))
        + cell(px + ivec2(-1, 0)) +                          cell(px + ivec2(1, 0))
        + cell(px + ivec2(-1, 1)) + cell(px + ivec2(0, 1)) + cell(px + ivec2(1, 1));
    int e = cell(px);
    FragColor = (((k == 2) && (e == 1)) || (k == 3)) ? 1.0 : 0.0;
}
)";

static const std::string shaderCodeConweysLifeVisualize_PS = R"(
#version 330 core

in vec2 uv;
uniform sampler2D texture0;

out vec4 FragColor;

#define TextureSize textureSize(texture0, 0).xy

float saturate(float x) { return clamp(x, 0., 1.); }

float edgeMark(vec2 uv, float fw) {
    vec2 edge = abs(fract(gl_FragCoord.xy + 0.5) * 2. - 1.);
    return smoothstep(fw * 2, 0, min(edge.x, edge.y));
}

void main() {
    vec3 f = texture2D(texture0, uv).rrr;
    vec2 fw = fwidth(uv) * TextureSize;
    f = mix(f, vec3(0, .6, 0), saturate(.03 / fw.x - 0.1) * edgeMark(uv, fw.x));
    // f.y = edgeMark(uv).x;
    FragColor = vec4(f, 1);
}

)";

class OpenGLApp2D : public Application {
    VisualObjectSprite2D m_spriteFullScreen, m_spriteBrush;
    GLShader m_shaderDefault2d, m_shaderLife, m_shaderVisualizeLife;

    GLFrameBuffer m_fb0, m_fb1; // buffers for convey's life

    std::shared_ptr<GLTexture2D> m_initialTexture = std::make_shared<GLTexture2D>(Image(projectDir / "resources" / "mona_liza.jpg"));
    std::shared_ptr<GLTexture2D> m_pixelBright = std::make_shared<GLTexture2D>(Image(glm::ivec2(1), 0xFF8800FF));
    std::shared_ptr<GLTexture2D> m_pixelDark = std::make_shared<GLTexture2D>(Image(glm::ivec2(1), 0x000000FF));

    CameraOrtho m_cameraView;
    int m_iterationIndex {};
    bool m_frameBufferFlip {};
    bool m_isPlaying {};

public:
    GLFrameBuffer& getThisFB() { return m_frameBufferFlip ? m_fb0 : m_fb1; }
    GLFrameBuffer& getThatFB() { return m_frameBufferFlip ? m_fb1 : m_fb0; }

    void initializeWithImage()
    {
        m_spriteFullScreen.setUniform("texture0", m_initialTexture);
        GLRenderManager::draw(&m_fb1, &m_shaderDefault2d, 0, &m_spriteFullScreen);
        m_spriteFullScreen.setUniform("texture0", m_fb1.getTexture());
        m_frameBufferFlip = false;
        m_iterationIndex = 0;
    }

    void updateLife()
    {
        m_spriteFullScreen.setUniform("texture0", getThisFB().getTexture());
        GLRenderManager::draw(&getThatFB(), &m_shaderLife, 0, &m_spriteFullScreen);
        m_spriteFullScreen.setUniform("texture0", getThatFB().getTexture());

        GLRenderParameters paramsAccumulate { GLBlend::OneMinusAlpha, GLDepth::Disabled };
        // GLRenderManager::draw(&m_frameBufferAccumulator, &m_shaderDefault2d, 0, &m_sprite, paramsAccumulate);

        m_frameBufferFlip = !m_frameBufferFlip;
        m_iterationIndex++;
    }

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

        // draw white on LMB down
        m_window.setMouseDownEvent(MouseButton::Left, [this](glm::ivec2 mousePos) {
            m_spriteBrush.setUniform("texture0", m_pixelBright);
            GLRenderManager::draw(&getThisFB(), &m_shaderDefault2d, 0, &m_spriteBrush);
        });

        // draw black on RMB down
        m_window.setMouseDownEvent(MouseButton::Right, [this](glm::ivec2 mousePos) {
            m_spriteBrush.setUniform("texture0", m_pixelDark);
            GLRenderManager::draw(&getThisFB(), &m_shaderDefault2d, 0, &m_spriteBrush);
        });

        // draw white on LMB move
        m_window.setMouseDragEvent(MouseButton::Left,
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_spriteBrush.setUniform("texture0", m_pixelBright);
                GLRenderManager::draw(&getThisFB(), &m_shaderDefault2d, 0, &m_spriteBrush);
            });

        // draw black on RMB move
        m_window.setMouseDragEvent(MouseButton::Right,
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_spriteBrush.setUniform("texture0", m_pixelDark);
                GLRenderManager::draw(&getThisFB(), &m_shaderDefault2d, 0, &m_spriteBrush);
            });

        // move brush on mouse move
        m_window.setMouseMoveEvent(MouseButton::Left,
            [this](glm::ivec2 currentScreenPos, glm::ivec2 delta) {
                glm::vec2 normalizedScreenPos = m_window.toNormalizedPos(currentScreenPos, true);
                glm::vec2 spritePosWorld = glm::inverse(m_cameraView.getViewMatrix()) * glm::vec4(normalizedScreenPos, 0.f, 1.f);
                spritePosWorld = glm::floor(spritePosWorld * 512.f) / 512.f;
                m_spriteBrush.setPos(spritePosWorld);
            });

        // reset on R
        m_window.addKeyDownEvent(SDLK_r, KMOD_NONE, [this]() { initializeWithImage(); });

        // update on space
        m_window.addKeyDownEvent(SDLK_SPACE, KMOD_NONE, [this]() {
            updateLife();
            m_isPlaying = false;
        });

        m_window.addKeyDownEvent(SDLK_p, KMOD_NONE, [this]() { m_isPlaying = true; });
        m_window.addKeyUpEvent(SDLK_p, KMOD_NONE, [this]() { m_isPlaying = false; });

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

        m_shaderDefault2d = GLShader(shaderCodeDefault2d_VS, shaderCodeDefault2d_PS);
        m_shaderLife = GLShader(shaderCodeDefault2d_VS, shaderCodeConweysLife_PS);
        m_shaderVisualizeLife = GLShader(shaderCodeDefault2d_VS, shaderCodeConweysLifeVisualize_PS);

        const int size = 1024;
        m_fb0.create({ size, size }, GLTexture2D::Format::R_8);
        m_fb1.create({ size, size }, GLTexture2D::Format::R_8);

        m_spriteBrush.setRectSize(glm::vec2(0), glm::vec2(2.f / size)); // set brush 1 px size
        m_spriteBrush.setUniform("opacity", 0.5f);

        initializeWithImage();
    }

    void updateWindow(float dt) override
    {
        if (m_isPlaying)
            for (int i = 0; i < 1; ++i)
                updateLife();

        // m_sprite.setUniform("texture0", m_frameBufferAccumulator.getTexture());

        m_window.clear();
        GLRenderManager::draw(&m_window, &m_shaderVisualizeLife, &m_cameraView, &m_spriteFullScreen);

        m_spriteBrush.setUniform("texture0", m_pixelBright);

        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_spriteBrush);
    }
};
typedef OpenGLApp2D App;
#endif // OPENGLAPP_LIFE_H
