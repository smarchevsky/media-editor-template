#ifndef GLSHADER_H
#define GLSHADER_H

#include <filesystem>

#include "gltexture.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <map>
#include <string>

namespace fs = std::filesystem;

/////////////// TO SEPARATE FILE
// static size_t constexpr const_hash(char const* input) { return *input ? static_cast<size_t>(*input) + 33 * const_hash(input + 1) : 5381; }

// struct AttributeName {
//     constexpr explicit AttributeName(const char* name)
//         : m_str(name)
//         , m_hash(const_hash(name))
//     {
//     }
//     constexpr AttributeName(const AttributeName& other) = default;
//     constexpr bool operator==(const AttributeName& rhs) const { return m_hash == rhs.m_hash; }
//     const char* m_str;
//     const size_t m_hash;
// };
//////////////////////////////
/////////////////////////////
//////////////////////////////

class GLShader;
class GLShaderInstance {
public:
    GLShaderInstance() = default;
    void setShader(GLShader* shader);
    template <class UniformType>
    bool setUniform(const char* name, UniformType var);
    void bind();

private:
    GLShader* m_shader {};
};

typedef std::map<std::string, int> UniformLocationMap;
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
    // bool createShaderProgram(const char* vertexShaderCode, const char* fragmentShaderCode);
    int getUniformLocation(const char* name);

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderInstance;
    friend class GLShaderCompiler;
};

template <class UniformType>
bool GLShaderInstance::setUniform(const char* name, UniformType var)
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

    static GLShader* getByName(const std::string& name);

private:
    GLShaderCompiler() = default;
    ~GLShaderCompiler() = default;

    static std::map<std::string, GLShader> s_staticShaders;
};

#endif // SHADER_H
