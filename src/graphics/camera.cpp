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
        camera->updateTransforms(shader);
    }
}

OrthoCamera::OrthoCamera()
{
}

void OrthoCamera::updateTransforms(GLShader* shader)
{
    if (m_isViewProjectionDirty) {
        m_isViewProjectionDirty = false;
        glm::vec2 scale = m_viewportSize * m_scaleMultiplier;
        glm::vec2 p0 = m_posWorld - scale * 0.5f;
        glm::vec2 p1 = m_posWorld + scale * 0.5f;
        // m_viewProjection = glm::ortho(p0.x, p1.x, p0.y, p1.y, -1.f, 1.f);
        m_viewProjection = glm::ortho(p0.x, p1.x, p1.y, p0.y, -1.f, 1.f);
    }
    if (shader) {
        shader->setUniform("view_matViewProjection", m_viewProjection);
    }
}
