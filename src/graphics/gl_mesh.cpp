#include "gl_mesh.h"

#include "model3d.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <iostream>

std::shared_ptr<GLMeshTriArray> GLMeshStatics::s_quad;
uint32_t GLMeshBase::s_currentBindedMeshHandle = 0;

//////////////////////// GL MESH BASE /////////////////////////

void GLMeshBase::bind() const
{
    if (s_currentBindedMeshHandle != m_VAO) {
        s_currentBindedMeshHandle = m_VAO;
        glBindVertexArray(m_VAO);
    }
}

//////////////////////// GL MESH TRI ARRAY /////////////////////////

void GLMeshTriArray::draw() const
{
    bind();
    glDrawArrays(GL_TRIANGLES, 0, m_vertCount);
}

GLMeshTriArray::~GLMeshTriArray()
{
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        m_VAO = 0;
    }
}

//////////////////////// GL MESH TRI INDICES /////////////////////////

void GLMeshTriIndices::draw() const
{
    bind();
    glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, 0);
}

GLMeshTriIndices::~GLMeshTriIndices()
{
    if (m_VAO) {
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

GLMeshTriIndices::GLMeshTriIndices(const Model3D& model)
{
    m_indicesCount = model.triangles.size() * 3;

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
        model.vertices.size() * sizeof(Vertex), model.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        model.triangles.size() * sizeof(glm::ivec3), model.triangles.data(), GL_STATIC_DRAW);

    //  (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    static constexpr int positionSize = sizeof(decltype(Vertex::position));
    static constexpr int normalSize = sizeof(decltype(Vertex::normal));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)positionSize);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(positionSize + normalSize));
    glEnableVertexAttribArray(2);
}

GLMeshStatics::GLMeshStatics()
{
    const float px0 = -1.f, px1 = 1.f;
    const float py0 = -1.f, py1 = 1.f;

    const float tx0 = 0.f, tx1 = 1.f;
    const float ty0 = 0.f, ty1 = 1.f;

    const float vertices[] = {

        px1, py1, tx1, ty1, // 11 right top
        px0, py1, tx0, ty1, // 01 left top
        px1, py0, tx1, ty0, // 10 right bottom

        px1, py0, tx1, ty0, // 10 right bottom
        px0, py1, tx0, ty1, // 01 left top
        px0, py0, tx0, ty0, // 00 left bottom
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    s_quad = std::make_shared<GLMeshTriArray>();
    s_quad->m_VAO = VAO;
    s_quad->m_VBO = VBO;
    s_quad->m_vertCount = 6;
}
