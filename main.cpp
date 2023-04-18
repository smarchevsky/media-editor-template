
#include "application.h"
#include "graphics/mesh.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <graphics/shader.h>

class OpenGLApp : public Application {

public:
    void init() override
    {
        Application::init();
    }
    void drawContext() override
    {
        glClearColor(0, 0.1f, 0.3f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        GL::MeshStatics::get().getQuad().draw();
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
