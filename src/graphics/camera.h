#ifndef CAMERA_H
#define CAMERA_H

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <string>

class GLShader;

class CameraBase {
public:
    virtual ~CameraBase() = default;
    virtual void updateUniforms(GLShader* shader) = 0;

    template <class CameraType>
    CameraType* as() { return dynamic_cast<CameraType*>(this); }
};

////////////////////////// CAMERA ORTHO //////////////////////////////

class CameraOrtho : public CameraBase {
protected:
    glm::mat4 m_matView = glm::mat4(1);

private:
    glm::vec2 m_posWorld = glm::vec2(0);
    glm::vec2 m_viewportSize = glm::vec2(1000, 1000);
    float m_scaleMultiplier = 0.003f;

    bool m_viewDirty = true;

public:
    void setScale(float scale) { m_scaleMultiplier = scale, m_viewDirty = true; }
    void multiplyScale(float scaleOffset) { m_scaleMultiplier *= scaleOffset, m_viewDirty = true; }
    void multiplyScaleOffseted(float scaleFactor, glm::vec2 viewSpaceOffset)
    {
        glm::vec2 mousePosOffset = (viewSpaceOffset - 0.5f * m_viewportSize)
            * m_scaleMultiplier;
        m_posWorld += mousePosOffset * (1 - scaleFactor);
        multiplyScale(scaleFactor);
    }

    void setViewportSize(glm::vec2 vs) { m_viewportSize = vs, m_viewDirty = true; }
    void setPosition_WS(glm::vec2 pos) { m_posWorld = pos, m_viewDirty = true; }
    void addOffset_WS(glm::vec2 offset) { m_posWorld += offset, m_viewDirty = true; }
    void addOffset_View(glm::vec2 offset) { addOffset_WS(offset * m_scaleMultiplier); }

    glm::vec2 getPos() const { return m_posWorld; }
    float getScale() const { return m_scaleMultiplier; }

    const glm::mat4& getView();

    virtual void updateUniforms(GLShader* shader) override;
};

////////////////////// CAMERA PERSPECTIVE //////////////////////////////

class CameraPerspective : public CameraBase {
public:
    void setFOV(float fov) { m_fov = fov, m_projectionDirty = true; }
    void setAR(float ar) { m_ar = ar, m_projectionDirty = true; }

    void setPos(glm::vec3 pos) { m_pos = pos, m_viewDirty = true; }
    void offsetPos(glm::vec3 delta_pos) { setPos(m_pos + delta_pos); }

    void setAim(glm::vec3 aim) { m_aim = aim, m_viewDirty = true; }
    void offsetAim(glm::vec3 delta_aim) { setAim(m_pos + delta_aim); }

    void setUp(glm::vec3 up) { m_up = up, m_viewDirty = true; }

    void setDistance(float newDistance)
    {
        auto offset = m_pos - m_aim;
        auto dir = offset / glm::length(offset);
        m_pos = m_aim + dir * newDistance;
        m_viewDirty = true;
    }

    glm::vec3 getPos() const { return m_pos; }
    glm::vec3 getAim() const { return m_aim; }
    glm::vec3 getUp() const { return m_up; }
    float getDistance() const { return glm::distance(m_pos, m_aim); }

    const glm::mat4& getView();
    const glm::mat4& getProjection();

    virtual void updateUniforms(GLShader* shader) override;

private:
    glm::mat4 m_matView;
    glm::mat4 m_matProjection;
    glm::vec3 m_pos = { 0.f, -10.f, 3.f }, m_aim = { 0.f, 0.f, 0.f }, m_up = { 0.f, 0.f, 1.f };
    float m_near = .1f, m_far = 2000.f;
    float m_fov = 1.f /*radians*/, m_ar = 1.f;
    bool m_viewDirty = true, m_projectionDirty = true;
};

#endif // ORTHOCAMERA_H
