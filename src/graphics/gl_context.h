#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <glm/vec3.hpp>
class GLContext {
public:
    static GLContext& getInit()
    {
        static GLContext instance;
        return instance;
    }

private:
    GLContext();
    ~GLContext();
};

#endif // GRAPHICS_COMMON_H
