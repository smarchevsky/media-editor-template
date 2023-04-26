#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

#include "gl_shader.h"

CameraBase::CameraBase()
{
}

void CameraBase::updateShaderUniforms(CameraBase* camera, GLShader* shader)
{
    shader->resetVariables(UniformDependency::View);
    if (camera) {
        camera->updateTransforms();
        shader->setUniform("view_matViewProjection", camera->m_viewProjection);
    }
}

OrthoCamera::OrthoCamera()
{
}

void OrthoCamera::updateTransforms()
{
    if (m_isDirty) {
        m_isDirty = false;
        glm::vec2 scale = m_viewportSize * m_scaleMultiplier;
        glm::vec2 p0 = m_posWorld - scale * 0.5f;
        glm::vec2 p1 = m_posWorld + scale * 0.5f;
        // m_viewProjection = glm::ortho(p0.x, p1.x, p0.y, p1.y, -1.f, 1.f);
        m_viewProjection = glm::ortho(p0.x, p1.x, p1.y, p0.y, -1.f, 1.f);
    }
}
