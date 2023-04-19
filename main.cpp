
#include "application.h"
#include "graphics/glmesh.h"
#include "graphics/graphics_common.h"

class OpenGLApp : public Application {

public:
    void init() override
    {
        Application::init();
    }
    void drawContext() override
    {
        GraphicsCommon::clear();

        GLMeshStatics::get().getQuad().draw();
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
