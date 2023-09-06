#include "visualobject.h"

#include <glm/gtc/matrix_transform.hpp>

void VisualObjectBase::draw()
{
    m_mesh->draw();
}

VisualObjectBase::~VisualObjectBase() { }

/////////////////////////// VisualObject2D /////////////////////

VisualObjectSprite2D::VisualObjectSprite2D()
{
    m_mesh = GLMeshStatics::get().getQuad2d();
}

const NameUniformMap& VisualObjectSprite2D::updateAndGetUniforms()
{
    if (m_dirty) {
        glm::mat4 mat(1);
        mat = glm::scale(mat, glm::vec3(m_size.x, m_size.y, 0.f));
        mat = glm::rotate(mat, m_angle, glm::vec3(0, 0, 1));
        mat[3][0] = m_pos.x;
        mat[3][1] = m_pos.y;

        setUniform(modelWorldName, mat);
        m_dirty = false;
    }
    return m_uniforms;
}

/////////////////////////////// MESH 3D ////////////////////////////

void VisualObject3D::setTransform(const glm::mat4& transform)
{
    setUniform(modelWorldName, transform);
}
