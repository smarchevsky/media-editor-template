#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

class GraphicsCommon {
public:
    static GraphicsCommon& getInit()
    {
        static GraphicsCommon instance;
        return instance;
    }


    static void setOpenGLViewport(int x, int y, int width, int height);

private:
    GraphicsCommon();
    ~GraphicsCommon();
};

#endif // GRAPHICS_COMMON_H
