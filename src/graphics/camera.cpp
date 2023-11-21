#include "camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;

CameraBase::~CameraBase() { }

// m_viewProjection = glm::ortho(p0.x, p1.x, p0.y, p1.y, -1.f, 1.f);
////////////////////////// CAMERA ORTHO //////////////////////////////

const NameUniformMap& CameraRect::updateAndGetUniforms()
{
    m_uniforms["cameraView"] = getViewMatrix();
    return m_uniforms;
}

const glm::mat4& CameraRect::getViewMatrix() const
{
    if (m_viewDirty) {
        m_viewMatrix = glm::ortho(m_rect[0].x, m_rect[1].x, m_rect[0].y, m_rect[1].y, -1.f, 1.f);
        m_viewDirty = false;
    }
    return m_viewMatrix;
}

const glm::mat4& CameraOrtho::getViewMatrix() const
{
    if (m_viewDirty) {
        glm::vec2 scale = m_viewportSize * m_scaleMultiplier;
        vec2 p0 = m_posWorld - scale * 0.5f;
        vec2 p1 = m_posWorld + scale * 0.5f;

        glm::vec2* rectMutable = const_cast<glm::vec2*>(m_rect);
        if (m_verticalFlip) {
            rectMutable[0] = vec2(p0.x, p1.y), rectMutable[1] = vec2(p1.x, p0.y);
        } else {
            rectMutable[0] = p0, rectMutable[1] = p1;
        }
    }

    return CameraRect::getViewMatrix();
}

////////////////////// CAMERA PERSPECTIVE //////////////////////////////

void CameraPerspective::updateView()
{
    if (m_viewDirty) {

        vec3 z = -glm::normalize(m_aimPosition - m_cameraPosition);
        vec3 x = -glm::normalize(glm::cross(z, m_up));
        vec3 y = -glm::cross(x, z);
        vec3 p = m_cameraPosition;

        m_cameraMatrix = glm::mat4(
            x.x, x.y, x.z, 0.f,
            y.x, y.y, y.z, 0.f,
            z.x, z.y, z.z, 0.f,
            p.x, p.y, p.z, 1.f);

        m_cameraMatrixInv = glm::inverse(m_cameraMatrix);

        // m_cameraViewMatrix = glm::lookAt(m_cameraPosition, m_aimPosition, m_up);
        // m_cameraMatrix = glm::inverse(m_cameraViewMatrix);

        m_viewDirty = false;
    }
}

void CameraPerspective::updateProjection()
{
    if (m_projectionDirty) {
        m_cameraProjection = glm::perspective(m_fov, m_ar, m_near, m_far);
        m_projectionDirty = false;
    }
}

const glm::mat4& CameraPerspective::getCameraMatrixInv()
{
    updateView();
    return m_cameraMatrixInv;
}

const glm::mat4& CameraPerspective::getCameraMatrix()
{
    updateView();
    return m_cameraMatrix;
}

const glm::mat4& CameraPerspective::getProjection()
{
    updateProjection();
    return m_cameraProjection;
}

const NameUniformMap& CameraPerspective::updateAndGetUniforms()
{
    m_uniforms["cameraView"] = getViewMatrix();
    m_uniforms["cameraPosition"] = m_cameraPosition;
    m_uniforms["cameraProjection"] = getProjection();
    return m_uniforms;
}

void CameraPerspective::pan(glm::vec2 delta)
{
    glm::mat4 matView = getCameraMatrix();

    float distance = glm::distance(m_cameraPosition, m_aimPosition);
    glm::vec3 right = matView[0];
    glm::vec3 up = matView[1];
    auto offset = (-right * delta.x + up * delta.y) * distance;

    m_cameraPosition += offset;
    setAim(m_aimPosition + offset);
}

void CameraPerspective::rotateAroundAim(glm::vec2 offsetDeltaRadians)
{
    m_sceneRotation += glm::vec2(-offsetDeltaRadians.x, offsetDeltaRadians.y);
    m_sceneRotation.y = glm::clamp(m_sceneRotation.y, -(float)M_PI_2 + 0.001f, (float)M_PI_2 - 0.001f);
    m_sceneRotation.x = fmodf(m_sceneRotation.x, M_PI * 2);

    glm::vec3 rotX = glm::rotateX(glm::vec3(0.f, getDistance(), 0.f), m_sceneRotation.y);
    glm::vec3 rotZ = m_aimPosition + glm::rotateZ(rotX, m_sceneRotation.x);
    m_cameraPosition = rotZ;
    m_viewDirty = true;
}

CameraPerspective::CameraPerspective()
{
    m_sceneRotation = glm::vec2(1.8f, .5f);
    m_cameraPosition = { -2.f, 0.f, 0.7f };
    m_aimPosition = { 0.f, 0.f, 0.7f };
    m_up = { 0.f, 0.f, 1.f };
    rotateAroundAim(glm::vec2(0));
}

////////////////////// CAMERA PERSPECTIVE JITTERED //////////////////////////////

const glm::mat4& CameraPerspectiveJittered::getProjection()
{
    glm::vec2 jitterAA = glm::diskRand(1.f) * m_jitterSizeAA;
    glm::vec2 jitterDOF = glm::diskRand(1.f) * m_jitterSizeDOF;
    // glm::vec4 j(jitter.x, jitter.y, 0.f, 0.f);
    m_cameraProjection = glm::perspective(m_fov, m_ar, m_near, m_far);
    auto projectedDistance = glm::vec4(0, 0, -getDistance(), 0) * m_cameraProjection;

    glm::vec4& z = m_cameraProjection[2];
    glm::vec4& w = m_cameraProjection[3];

    z.x += jitterDOF.x / projectedDistance.z;
    z.y += jitterDOF.y / projectedDistance.z;
    w.x += jitterDOF.x;
    w.y += jitterDOF.y;

    z.x += jitterAA.x;
    z.y += jitterAA.y;

    m_projectionDirty = true;
    return m_cameraProjection;
}
