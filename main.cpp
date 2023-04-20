
#include "application.h"
#include "graphics/graphics_common.h"
#include "graphics/sprite2d.h"

class OpenGLApp : public Application {
    std::vector<Sprite2d> m_sprites;

public:
    void init() override
    {
        Application::init();
        Sprite2d s;
        s.setPos({ 0.5f, 0.f });
        //s.setSize({ 0.5f, 0.5f });
        m_sprites.push_back(s);
    }

    void drawContext() override
    {
        GraphicsCommon::clear();
        for (auto& s : m_sprites)
            s.draw();
        // GLMeshStatics::get().getQuad2d().draw();
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
