#ifndef ENTITY_H
#define ENTITY_H

#include "gl_shader.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class EntityBase {
public:
protected:
    NameUniformMap m_uniforms;

public:
    void setUniform(HashString str, const UniformVariant& var) { m_uniforms[str] = var; }

    virtual const NameUniformMap& updateAndGetUniforms() { return m_uniforms; }

    virtual ~EntityBase() = 0;
};

inline EntityBase::~EntityBase() { }

#endif // ENTITY_H
