#include "application.h"
#include <memory>
//////////////////////////////////////////////////

int main(int argc, char** argv)
{
    Application app;
    app.init();
    app.mainLoop();
    // Application_UVBSP uvbspApp;
    // uvbspApp.mainLoop();

    return 0;
}
