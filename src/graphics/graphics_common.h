#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include "glshader.h"
class GraphicsCommon {
public:
    static GraphicsCommon& getInit()
    {
        static GraphicsCommon instance;
        return instance;
    }

    static void setOpenGLViewport(int x, int y, int width, int height);
    static void clear();


private:
    GraphicsCommon();
    ~GraphicsCommon();
};

#endif // GRAPHICS_COMMON_H
