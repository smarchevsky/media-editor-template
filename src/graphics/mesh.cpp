#include "mesh.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

namespace GL {
Mesh MeshStatics::s_quad;

void Mesh::draw()
{
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_vertCount);
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

MeshStatics::MeshStatics()
{
    const float px0 = -1., px1 = 1;
    const float py0 = -1., py1 = 1;

    const float tx0 = 0, tx1 = 1;
    const float ty0 = 0, ty1 = 1;
    // clang-format off
    const float vertices[] = {

        px1, py1, tx1, ty1, // 11
        px0, py1, tx0, ty1, // 01
        px1, py0, tx1, ty0, // 10

        px1, py0, tx1, ty0, // 10
        px0, py1, tx0, ty1, // 01
        px0, py0, tx0, ty0, // 00
    };
    // clang-format on

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    s_quad.m_VAO = VAO;
    s_quad.m_VBO = VBO;
    s_quad.m_vertCount = 6;
}
}
