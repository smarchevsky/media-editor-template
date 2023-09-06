#ifndef ENTITY_H
#define ENTITY_H

#include "gl_shader.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class EntityBase {
public:
    typedef std::unordered_map<HashString, UniformVariant> NameUniformMap;

protected:
    NameUniformMap m_uniforms;

public:
    void setUniform(HashString str, const UniformVariant& var) { m_uniforms[str] = var; }

    const NameUniformMap& getUniforms() { return m_uniforms; }

    virtual void applyUniforms(GLShader* shader)
    {
        for (const auto& u : getUniforms())
            shader->setUniform(u.first, u.second);
    }

    virtual ~EntityBase() = 0;
};

inline EntityBase::~EntityBase() { }

#endif // ENTITY_H
