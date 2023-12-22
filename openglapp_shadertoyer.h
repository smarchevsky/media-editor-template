#ifndef OPENGLAPP_SHADERTOYER_H
#define OPENGLAPP_SHADERTOYER_H

#include "application.h"

#include "graphics/gl_shader.h"
#include "graphics/visualobject.h"

#include "graphics/gl_rendermanager.h"
#include <thread>

// #define SAVE_IMAGE_SEQUENCE

#ifdef SAVE_IMAGE_SEQUENCE
#include "thread_pool.h"
#endif

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

static const std::string dummy_VS = R"( // vertex shader for screen quad, does nothing
#version 330 core
layout (location = 0) in vec3 position;
void main(){ gl_Position = vec4(position, 1.0); }
)";

static const std::string onScreenWithTransparency_FS = R"( // vertex shader for screen quad, does nothing
#version 330 core

uniform sampler2D texture0;
uniform vec2 iResolution;
out vec4 FragColor;

void main()
{
    vec2 uv = vec2(gl_FragCoord.xy) / iResolution;
    vec4 t = texture2D(texture0, uv);

    vec2 p = floor(10 * gl_FragCoord.xy * vec2(iResolution.x / iResolution.y, 1) / iResolution.xy);
    float f = mod(p.x + p.y, 2.);
    t.rgb = mix(vec3(0.3 + 0.4 * f), t.rgb, t.a);

    //t.rgb = vec3(t.a);

    FragColor = t;
}
)";
static const int steps = 16;
static const double renderTime = 20.0; // seconds
static const glm::vec2 SIZE = glm::vec2(512, 512);

class OpenGLApp_Shadertoyer : public Application {

    VisualObjectSprite2D m_spriteDraw3D, m_spriteAccumulator;
    GLShader m_shaderShadertoy, m_shaderDefault2D;
    GLShader m_shaderShowOnScreenWithTransparency;
    CameraRect m_cameraRect;

    GLFrameBuffer m_frameBufferAccumulator;

    int m_frameIndex = 32;
    int m_frameIndexMax = 33;
    double m_time = m_frameIndex / 30.f;
    int m_updateStep = 0;
    glm::vec2 m_mousePos;
    bool m_isLMBdown {}, m_isRMBdown {};

#ifdef SAVE_IMAGE_SEQUENCE
    ThreadPool m_threadPool;
#endif

public:
    void init() override
    {
        m_cameraRect.setViewRect(glm::vec4(0, 0, 1, 1));
        m_window.setSize(SIZE);

        m_window.setMouseUpEvent(MouseButton::Left, [this](glm::ivec2 mousePos) { m_isLMBdown = false; });

        m_shaderShadertoy = GLShader(
            textFromFile(shaderDir / "default2d.vert"),
            textFromFile(shaderDir / "shadertoy.frag"));

        m_shaderDefault2D = GLShader::FromFile("default2d.vert", "default2d.frag");
        m_shaderShowOnScreenWithTransparency = GLShader(dummy_VS, onScreenWithTransparency_FS);

        // Image image(glm::ivec2(1024), TexelFormat::RGB_8);
        // for (int i = 0; i < image.getDataSize(); ++i) {
        //     image.getDataMutable()[i] = rand() % 256;
        // }
        // auto noiseTex = std::make_shared<GLTexture2D>(image);

        m_frameBufferAccumulator.create(m_window.getSize(), TexelFormat::RGBA_32F);
        m_frameBufferAccumulator.setClearColor(glm::vec4(0));
        m_frameBufferAccumulator.clear();

        m_spriteDraw3D.setUniform("iResolution", glm::vec2(m_window.getSize()));
        m_spriteDraw3D.setRectSize(glm::vec2(0), glm::vec2(1.f / steps));

        m_spriteAccumulator.setUniform("iResolution", glm::vec2(m_window.getSize()));
        m_spriteAccumulator.setUniform("texture0", m_frameBufferAccumulator.getTexture());
    }

    static void writeToFile(const Image& image, const std::string& path)
    {
// #define DUMMY_SAVE
#ifdef DUMMY_SAVE
        std::this_thread::sleep_for(std::chrono::milliseconds(300 + rand() % 400));
        LOG("succesfully written to: " << path);
#else
        if (image.writeToFile(path, true)) {
            LOG("succesfully written to: " << path);
        } else {
            LOGE("could not write file");
        }
#endif
        /*
            uniform vec3      iResolution;           // viewport resolution (in pixels)
            uniform float     iTime;                 // shader playback time (in seconds)
            uniform float     iTimeDelta;            // render time (in seconds)
            uniform float     iFrameRate;            // shader frame rate
            uniform int       iFrame;                // shader playback frame
            uniform float     iChannelTime[4];       // channel playback time (in seconds)
            uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
            uniform vec4      iMouse;
            mouse pixel coords. xy: current (if MLB down), zw: click
        */
    }

    void updateSpritePos(int index)
    {
        assert(index < steps * steps);
        glm::vec2 ipos(index % steps, index / steps);
        m_spriteDraw3D.setPos(ipos / float(steps));
    }

    void updateWindow(float dt) override
    {
        dt = 1 / 30.f;
        updateSpritePos(m_updateStep);
        m_spriteDraw3D.setUniform("iTime", float(m_time));
        m_spriteDraw3D.setUniform("iFrame", int(m_frameIndex));
        m_spriteDraw3D.setUniform("iTimeDelta", dt);
        m_spriteDraw3D.setUniform("iMouse", glm::vec4(m_mousePos.x, m_mousePos.y, m_isLMBdown, 0));

        GLRenderManager::draw(&m_frameBufferAccumulator, &m_shaderShadertoy, &m_cameraRect, &m_spriteDraw3D);

        GLRenderManager::draw(&m_window, &m_shaderShowOnScreenWithTransparency, nullptr, &m_spriteAccumulator);

        bool mustClear = false;
        if (m_updateStep == steps * steps - 1) {
            mustClear = true;
#ifdef SAVE_IMAGE_SEQUENCE
            // write file async
            Image image(m_frameBufferAccumulator.getBufferImage(TexelFormat::RGBA_8));
            std::string path = std::string("render_result/image_") + std::to_string(m_frameIndex) + ".png";
            m_threadPool.queue(std::bind(writeToFile, std::move(image), path));
            // writeToFile(image, path);

            if (m_frameIndex >= (m_frameIndexMax - 1))
                m_window.exit();
            LOG("");
#endif
            ++m_frameIndex;
            m_time += dt;
            m_updateStep = -1;
        }

        if (mustClear) {
            m_frameBufferAccumulator.clear();
        }
        m_updateStep++;
    }
    ~OpenGLApp_Shadertoyer()
    {
        LOG("Finished at time: " << m_time << ", frame: " << m_frameIndex);
    }
};
typedef OpenGLApp_Shadertoyer App;
#endif // OPENGLAPP_SHADERTOYER_H
