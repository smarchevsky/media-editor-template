
#include "application.h"
#include "graphics/gl_context.h"
#include "graphics/sprite2d.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
class OpenGLApp : public Application {
    std::vector<Sprite2d> m_sprites;
    // GLFrameBuffer fb;

public:
    void init() override
    {
        Application::init();
        GLTextureInstance texChecker(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
        GLTextureInstance texLiza(Image(projectDir / "resources" / "mona_liza.jpg"));
        // m_textureDefault.fromImage(Image({ 128, 128 }, glm::ivec4(100, 200, 255, 255)));
        {
            Sprite2d s;
            s.init();
            s.setPos({ 0.5f, 0.f });
            s.setRotation(0.2f);

            s.getShaderInstance().updateUniform("texture0", texChecker);
            s.getShaderInstance().updateUniform("texture1", texLiza);

            m_sprites.push_back(s);
        }
        {
            Sprite2d s;
            s.init();
            s.setPos({ -0.5f, -0.3f });

            s.getShaderInstance().updateUniform("texture0", texLiza);
            s.getShaderInstance().updateUniform("texture1", texChecker);

            m_sprites.push_back(s);
        }
        // fb.create({ 512, 512 });
    }

    void drawContext() override
    {
        GLContext::clear();
        // m_texture0.bind();

        for (auto& s : m_sprites) {
            s.draw(*m_window);
        }
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
