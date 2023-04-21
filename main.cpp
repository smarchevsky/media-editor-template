
#include "application.h"
#include "graphics/gl_context.h"
#include "graphics/sprite2d.h"

class OpenGLApp : public Application {
    std::vector<Sprite2d> m_sprites;
    GLFrameBuffer fb;

public:
    void init() override
    {
        Application::init();

        for (int i = 0; i < 4; ++i) {
            Sprite2d s;
            s.setPos({ 0.5f, 0.f });
            m_sprites.push_back(Sprite2d());
        }
        fb.create({ 512, 512 });
    }

    void drawContext() override
    {
        GLContext::clear();
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
