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
#include <variant>
#include <iostream>

namespace fs = std::filesystem;

class GLShader;

typedef std::unordered_map<HashString, int> UniformLocationMap;
class GLShader : NoCopy<GLShader> {
private: // DATA
    uint32_t m_shaderProgram {};
    UniformLocationMap m_uniforms;

public:
    GLShader() = default;
    ~GLShader();
    GLShader(GLShader&& r);
    int getHandle() const { return m_shaderProgram; }

    template <class UniformType>
    void setUniform(HashString name, UniformType var)
    {
        int location = getUniformLocation(name);
        if (location != -1) {
            bind();
            setUniform(location, var);
        }
        else {
            LOGE("Invalid name");
        }
    }

private:
    void bind();
    void initialize(uint32_t shaderProgram, const UniformLocationMap& uniforms) { m_shaderProgram = shaderProgram, m_uniforms = uniforms; }

    void setUniform(int location, float var);
    void setUniform(int location, const glm::vec2& var);
    void setUniform(int location, const glm::vec3& var);
    void setUniform(int location, const glm::vec4& var);
    void setUniform(int location, const glm::mat4& var);
    void setUniform(int location, const GLTexture& texture);

    int getUniformLocation(HashString name);

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderManager;
};

class GLShaderManager {

public:
    static GLShaderManager& get()
    {
        static GLShaderManager shaderGeneratorStatic;
        return shaderGeneratorStatic;
    }
    GLShader* addShader(
        const char* uniqueName,
        const char* vertexShaderCode,
        const char* fragmentShaderCode);

    GLShader* addShader(
        const char* uniqueName,
        const fs::path& vertexShaderPath,
        const fs::path& fragmentShaderPath);

    GLShader* getDefaultShader2d();

    GLShader* getByName(HashString str);

private:
    GLShaderManager() = default;
    ~GLShaderManager() = default;

    static std::unordered_map<HashString, GLShader> s_staticShaders;
};

#endif // SHADER_H
