
#include "application.h"
#include <GL/gl.h>

class OpenGLApp : public Application {
    void drawContext() override
    {
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};

int main()
{
    OpenGLApp app;
    app.init();
    app.mainLoop();

    return 0;
}
