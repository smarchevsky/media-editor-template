#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H


class GLContext {
public:
    static GLContext& getInit()
    {
        static GLContext instance;
        return instance;
    }

    static void setOpenGLViewport(int x, int y, int width, int height);
    static void clear();


private:
    GLContext();
    ~GLContext();
};

#endif // GRAPHICS_COMMON_H
