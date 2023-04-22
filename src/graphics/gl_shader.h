#ifndef GLSHADER_H
#define GLSHADER_H

#include <filesystem>

#include "gl_texture.h"
#include "hashstring.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fs = std::filesystem;
class GLShader;

enum class UniformType : uint8_t {
    Invalid,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat4,
    Texture2D,
    InvalidMax
};

//////////////////////// SHADER //////////////////////////
struct UniformInfo {
    std::string name;
    UniformType type = UniformType::InvalidMax;
    uint8_t textureIndex; // texture index, if texture
};
typedef std::vector<UniformInfo> UniformList;

typedef std::variant<
    char, // is invalid, dont set char :)
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4,
    std::shared_ptr<GLTexture>>
    UniformVariant;

class GLShader : NoCopy<GLShader> {
public:
    GLShader() = default;
    ~GLShader();
    GLShader(GLShader&& r);
    int getHandle() const { return m_shaderProgram; }

    void setUniform(const char* name, const UniformVariant& var, UniformType type)
    {
        bind();
        int location = getUniformLocation(name);
        if (location != -1) {
            setUniform(location, var, type);
        } else {
            LOGE("Invalid name");
        }
    }
    void setUniform(int location, const UniformVariant& var, UniformType type);

    const UniformList& getUniforms() { return m_uniforms; }

    void bind();
private:
    void initialize(uint32_t shaderProgram, const std::vector<UniformInfo>& uniforms)
    {
        m_shaderProgram = shaderProgram;
        m_uniforms = uniforms;
    }

    int getUniformLocation(const char* name);

private: // DATA
    uint32_t m_shaderProgram {};
    UniformList m_uniforms;

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderManager;
};

//////////////////////// SHADER INSTANCE //////////////////////////

struct UniformData {
    UniformInfo info;
    int location;
    UniformVariant dataVariant;
};

typedef std::unordered_map<HashString, UniformData> UniformDataList;

class GLShaderInstance {
public:
    void setShader(GLShader* shader) { m_shader = shader; }
    void trackUniformsExcept(std::initializer_list<std::string> names)
    {
        const auto& uniforms = m_shader->getUniforms();
        if (!m_shader)
            return;

        for (int i = 0; i < uniforms.size(); ++i) {
            const UniformInfo& uniformInfo = uniforms[i];

            const std::string& uniformName = uniformInfo.name;

            bool dontAdd = false;
            for (const auto& n : names) {
                if (uniformName == n) {
                    dontAdd = true;
                    break;
                }
            }
            if (dontAdd)
                continue;

            UniformData data;
            data.info = uniformInfo;
            data.location = i;

            HashString uniformHashString(uniformName.c_str());

            m_savedUniforms.insert({ uniformHashString, data });
        }
    }

    void updateUniform(HashString name, const UniformVariant& var);
    UniformVariant getUniform(HashString name);

    void applyUniformData() const
    {
        if (!m_shader)
            return;

        m_shader->bind();
        for (const auto& u : m_savedUniforms) {
            const UniformData& data = u.second;
            m_shader->setUniform(data.location, data.dataVariant, data.info.type);
        }
    }

private:
    GLShader* m_shader {};
    UniformDataList m_savedUniforms;
};

//////////////////////// SHADER MANAGER //////////////////////////

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
