
#include "application.h"

#include "graphics/gl_shader.h"
#include "graphics/sprite2d.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

class OpenGLApp : public Application {
    std::vector<Sprite2d> m_sprites;
    GLFrameBuffer m_fb;

public:
    void init() override
    {
        Application::init();
        m_fb.create({ 2048, 2048 });
        m_fb.getTexture()->setFiltering(GLTexture::Filtering::Linear);
        m_fb.getTexture()->setWrapping(GLTexture::Wrapping::ClampEdge);
        auto texChecker = SHARED_TEXTURE(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
        // auto texLiza = SHARED_TEXTURE(Image(projectDir / "resources" / "mona_liza.jpg"));
        //  m_textureDefault.fromImage(Image({ 128, 128 }, glm::ivec4(100, 200, 255, 255)));
        {
            Sprite2d s; // 0
            s.setPos({ 0.5f, 0.f });
            s.getShaderInstance().set("texture0", texChecker);
            m_sprites.push_back(s);
        }
        {
            Sprite2d s; // 1
            s.setPos({ -0.5f, -0.3f });
            s.getShaderInstance().set("texture0", m_fb.getTexture());
            m_sprites.push_back(s);
        }
        // fb.create({ 512, 512 });
    }

    void updateWindow(float dt) override
    {
        m_fb.clear(0, 0, 0, 1);

        GLShaderManager::get().getDefaultShader2d()->setUniform(
            "view_matViewProjection", glm::mat4(1));

        m_sprites[0].draw(m_fb);
        m_sprites[0].addRotation(dt * 0.1f);

        m_window.clear(0.16f, 0.16f, 0.16f, 1);
        glm::mat4 viewProjection;

        GLShaderManager::get().getDefaultShader2d()->setUniform(
            "view_matViewProjection",
            m_camera.getViewProjection());

        m_sprites[1].draw(m_window);
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
