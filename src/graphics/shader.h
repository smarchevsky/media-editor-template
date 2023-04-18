#ifndef SHADER_H
#define SHADER_H

#include <filesystem>
#include <glm/glm.hpp>
#include <string>
namespace GL {
namespace fs = std::filesystem;
class Shader {
    int m_shaderProgramHandle {};
    std::string m_shaderVisualName;

public:
    void create(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath);
    void create(const char* vs, const char* fs, const char* shaderVisialName = nullptr);

    ~Shader();
    void bind();

    void setUniform(const char* name, float var);
    void setUniform(const char* name, const glm::vec2& var);
    void setUniform(const char* name, const glm::vec3& var);
    void setUniform(const char* name, const glm::vec4& var);
    void setUniform(const char* name, const glm::mat4& var);

    int getProgram() const { return m_shaderProgramHandle; }

private:
    void compileShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
    int getUniformLocation(const char* name);
    int createShader(const char *shaderSource, int shaderType);
};
}
#endif // SHADER_H
