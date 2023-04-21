
#include "application.h"
#include "graphics/gl_context.h"
#include "graphics/sprite2d.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
class OpenGLApp : public Application {
    std::vector<Sprite2d> m_sprites;
    GLFrameBuffer fb;
    GLTexture m_textureDefault;

public:
    void init() override
    {
        Application::init();

        m_textureDefault.fromImage(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
        // m_textureDefault.fromImage(Image({ 128, 128 }, glm::ivec4(100, 200, 255, 255)));

        Sprite2d s;
        s.setPos({ 0.5f, 0.f });
        m_sprites.push_back(Sprite2d());

        fb.create({ 512, 512 });
    }

    void drawContext() override
    {
        GLContext::clear();
        m_textureDefault.bind();
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
