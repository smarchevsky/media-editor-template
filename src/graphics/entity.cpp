#include "entity.h"
#include "gl_shader.h"
#include <glm/gtc/matrix_transform.hpp>

void EntityBase::setUniform(HashString str, const UniformVariant& var)
{
    auto it = m_uniforms.find(str);
    if (it != m_uniforms.end()) {
        it->second = var;
    } else {
        assert(false && "No such uniform");
    }
}

void EntityBase::applyUniforms(GLShader* shader)
{
    for (const auto& u : getUniforms())
        shader->setUniform(u.first, u.second);
}

/////////////////////////////// SPRITE 2D ////////////////////////////

EntitySprite2D::EntitySprite2D()
    : m_meshQuad(GLMeshStatics::get().getQuad2d())
{
    initializeUniform("modelWorld", glm::mat4(1));
    initializeUniform("texture0", Texture2Ddata());
}

void EntitySprite2D::applyUniformsAndDraw(GLShader* shader)
{
    if (m_dirty) {
        glm::mat4 mat(1);
        mat = glm::scale(mat, glm::vec3(m_size.x, m_size.y, 0.f));
        mat = glm::rotate(mat, m_angle, glm::vec3(0, 0, 1));
        mat[3][0] = m_pos.x;
        mat[3][1] = m_pos.y;

        setUniform("modelWorld", mat);
        m_dirty = false;
    }

    applyUniforms(shader);
    m_meshQuad.draw();
}

/////////////////////////////// MESH 3D ////////////////////////////

EntityMesh3D::EntityMesh3D()
{
    initializeUniform("modelWorld", glm::mat4(1));
    initializeUniform("texture0", Texture2Ddata());
}

void EntityMesh3D::setTransform(const glm::mat4& transform)
{
    setUniform("modelWorld", transform);
}

void EntityMesh3D::applyUniformsAndDraw(GLShader* shader)
{
    applyUniforms(shader);
    m_mesh->draw();
}