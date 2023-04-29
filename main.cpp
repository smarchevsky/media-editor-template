
#include "application.h"

#include "graphics/drawable.h"
#include "graphics/gl_shader.h"

#include "graphics/gl_rendermanager.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

class OpenGLApp : public Application {
    std::vector<Sprite2d> m_sprites;
    std::shared_ptr<GLShader> m_shaderDefault2d;
    GLFrameBuffer m_fb;

public:
    void init() override
    {
        Application::init();
        m_shaderDefault2d = GLShaderManager::get().getDefaultShader2d();

        m_fb.create({ 2048, 2048 });
        m_fb.getTexture()->setFiltering(GLTexture::Filtering::LinearMipmap);
        m_fb.getTexture()->setWrapping(GLTexture::Wrapping::ClampEdge);
        auto texChecker = SHARED_TEXTURE(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
        auto texLiza = SHARED_TEXTURE(Image(projectDir / "resources" / "mona_liza.jpg"));
        //  m_textureDefault.fromImage(Image({ 128, 128 }, glm::ivec4(100, 200, 255, 255)));
        {
            Sprite2d s; // 0
            s.setPos({ 0.5f, 0.f });
            s.setUniform("texture0", texChecker);
            s.setUniform("texture1", texLiza);
            m_sprites.push_back(s);
        }
        {
            Sprite2d s; // 1
            s.setPos({ -0.5f, -0.3f });
            s.setUniform("texture0", texChecker);
            s.setUniform("texture1", m_fb.getTexture());
            m_sprites.push_back(s);
        }
        // fb.create({ 512, 512 });
    }

    void updateWindow(float dt) override
    {
        GLRenderManager rm;
        m_sprites[0].addRotation(dt * 0.1f);

        m_fb.clear(0, 0, 0, 1);
        rm.draw(*m_shaderDefault2d, m_fb, nullptr, &m_sprites[0]);

        m_window.clear(0.16f, 0.16f, 0.16f, 1);
        rm.draw(*m_shaderDefault2d, m_window, m_camera.get(), &m_sprites[1]);
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
