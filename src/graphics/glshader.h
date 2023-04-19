#ifndef GLSHADER_H
#define GLSHADER_H

#include <filesystem>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "gltexture.h"
#include <string>

namespace fs = std::filesystem;
class GLShader {
    int m_shaderProgramHandle {};
    std::string m_shaderVisualName;

public:
    void create(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath);
    void create(const char* vs, const char* fs, const char* shaderVisialName = nullptr);

    ~GLShader();
    void bind();

    void setUniform(const char* name, float var);
    void setUniform(const char* name, const glm::vec2& var);
    void setUniform(const char* name, const glm::vec3& var);
    void setUniform(const char* name, const glm::vec4& var);
    void setUniform(const char* name, const glm::mat4& var);
    void setUniform(const char* name, const GLTexture& texture);

    int getHandle() const { return m_shaderProgramHandle; }

private:
    void compileShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
    int getUniformLocation(const char* name);
    int createShader(const char* shaderSource, int shaderType);
};

#endif // SHADER_H
