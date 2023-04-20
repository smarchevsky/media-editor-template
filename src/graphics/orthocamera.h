#ifndef ORTHOCAMERA_H
#define ORTHOCAMERA_H

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class OrthoCamera {
    glm::mat4 m_viewProjection;

private:
    glm::vec2 m_posWorld = glm::vec2(0);
    glm::vec2 m_viewportSize = glm::vec2(1000, 1000);
    float m_scaleMultiplier = 0.003f;

    bool m_isDirty = true;

public:
    OrthoCamera();

    // clang-format off
    void setScale(float scale)            { m_scaleMultiplier = scale,        m_isDirty = true; }
    void multiplyScale(float scaleOffset) { m_scaleMultiplier *= scaleOffset, m_isDirty = true; }
    void multiplyScaleOffseted(float scaleFactor, glm::vec2 viewSpaceOffset)
    {
        glm::vec2 mousePosOffset = (viewSpaceOffset - 0.5f * m_viewportSize) * m_scaleMultiplier;
        m_posWorld += mousePosOffset * (1 - scaleFactor);
        multiplyScale(scaleFactor);
    }
    void setViewportSize(glm::vec2 vs)    { m_viewportSize = vs,              m_isDirty = true; }
    void setPosition_WS(glm::vec2 pos)    { m_posWorld = pos,                 m_isDirty = true; }
    void addOffset_WS(glm::vec2 offset)   { m_posWorld += offset,             m_isDirty = true; }

    void addOffset_View(glm::vec2 offset) {addOffset_WS(offset * m_scaleMultiplier); }

    glm::vec2 getPos()        const       { return m_posWorld; }
    float getScale()          const       { return m_scaleMultiplier; }
    // clang-format on

    //    glm::mat4 getViewProjection();
    bool getViewProjection(glm::mat4& outViewProjection);
};

#endif // ORTHOCAMERA_H