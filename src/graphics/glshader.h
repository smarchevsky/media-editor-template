#ifndef GLSHADER_H
#define GLSHADER_H

#include <filesystem>

#include "gltexture.h"
#include "hashstring.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

class GLShader;
class GLShaderInstance {
public:
    GLShaderInstance() = default;
    void setShader(GLShader* shader);

    template <class UniformType>
    bool setUniform(HashString name, UniformType var);
    void bind();

private:
    GLShader* m_shader {};
};

typedef std::unordered_map<HashString, int> UniformLocationMap;
class GLShader : NoCopy<GLShader> {
private: // DATA
    uint32_t m_shaderProgram {};
    UniformLocationMap m_uniforms;

public:
    GLShader() = default; // does nothing, create from GLShaderCompiler
    ~GLShader();
    GLShader(GLShader&& r);

private:
    int getHandle() const
    {
        return m_shaderProgram;
    }
    void bind();
    void initialize(uint32_t shaderProgram, const UniformLocationMap& uniforms)
    {
        m_shaderProgram = shaderProgram;
        m_uniforms = uniforms;
    }
    void setUniform(int location, float var);
    void setUniform(int location, const glm::vec2& var);
    void setUniform(int location, const glm::vec3& var);
    void setUniform(int location, const glm::vec4& var);
    void setUniform(int location, const glm::mat4& var);
    void setUniform(int location, const GLTexture& texture);

    int getUniformLocation(HashString name);

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderInstance;
    friend class GLShaderCompiler;
};

template <class UniformType>
bool GLShaderInstance::setUniform(HashString name, UniformType var)
{
    if (!m_shader)
        return false;
    auto location = m_shader->getUniformLocation(name);
    if (location != -1) {
        m_shader->setUniform(location, var);
        return true;
    }
    return false;
}

inline void GLShaderInstance::bind()
{
    if (m_shader)
        m_shader->bind();
}

class GLShaderCompiler {

public:
    static GLShaderCompiler& get()
    {
        static GLShaderCompiler shaderGeneratorStatic;
        return shaderGeneratorStatic;
    }
    GLShader* addShader(
        const char* vertexShaderCode,
        const char* fragmentShaderCode,
        const char* uniqueName);

    GLShader* addShader(
        const fs::path& vertexShaderPath,
        const fs::path& fragmentShaderPath,
        const char* uniqueName);

    GLShader* getDefaultShader2d();

    static GLShader* getByName(HashString str);

private:
    GLShaderCompiler() = default;
    ~GLShaderCompiler() = default;

    static std::unordered_map<HashString, GLShader> s_staticShaders;
};

#endif // SHADER_H
