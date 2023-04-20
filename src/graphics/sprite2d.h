#ifndef SPRITE2D_H
#define SPRITE2D_H

#include "glmesh.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
class GLShader;

class Sprite2d // square sprite -1 to 1
{
    const GLMesh& m_mesh;
    GLShader* m_shader;

    glm::mat4 m_transform;

public:
    Sprite2d();
    Sprite2d& setPos(glm::vec2 pos);
    Sprite2d& setSize(glm::vec2 size);
    void draw();
};

#endif // SPRITE2D_H
