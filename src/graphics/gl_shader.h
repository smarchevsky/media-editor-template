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

typedef std::variant<
    char, // is invalid, dont set char :)
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4,
    std::shared_ptr<GLTexture>>
    UniformVariant;

enum class UniformDependency : uint8_t {
    Object, // object-related variables
    View, // view-related variables
    Free // non-related shader-wise variables
};
struct UniformInfo {
    std::string name;
    UniformType type = UniformType::InvalidMax;
    UniformDependency dependency = UniformDependency::Object;
    uint8_t textureIndex; // texture index, if texture
};
typedef std::vector<UniformInfo> UniformList;

//////////////////////// UniformData //////////////////////////

struct UniformData {
    UniformInfo info;
    int location;
    UniformVariant data;
};
typedef std::unordered_map<HashString, UniformData> UniformDataList;

//////////////////////// SHADER //////////////////////////

class GLShader : NoCopy<GLShader> {
public:
    class Instance {
    protected:
    public:
        void updateUniform(HashString name, const UniformVariant& var);
        UniformVariant getUniform(HashString name);

        Instance() = default;
        Instance(GLShader* shader, UniformDependency dependency)
        {
            m_shader = shader;
            const auto& uniforms = m_shader->getUniforms();

            for (int i = 0; i < uniforms.size(); ++i) {
                const UniformInfo& uniformInfo = uniforms[i];

                if (uniformInfo.dependency == dependency) {
                    const std::string& uniformName = uniformInfo.name;

                    UniformData data;
                    data.info = uniformInfo;
                    data.location = i;

                    HashString uniformHashString(uniformName.c_str());
                    m_savedUniforms.insert({ uniformHashString, data });
                }
            }
        }

        void applyUniformData() const
        {
            m_shader->bind();
            for (const auto& u : m_savedUniforms) {
                const UniformData& d = u.second;
                m_shader->setUniform(d.location, d.data, d.info.type, d.info.textureIndex);
            }
        }

    private:
        GLShader* m_shader {};
        UniformDataList m_savedUniforms;
    };
    Instance getInstance(UniformDependency dependency)
    {
        return Instance(this, dependency);
    }

    GLShader() = default;
    ~GLShader();
    GLShader(GLShader&& r);
    int getHandle() const { return m_shaderProgram; }

    void setUniform(const char* name, const UniformVariant& var, UniformType type, int textureIndex)
    {
        bind();
        int location = getUniformLocation(name);
        if (location != -1) {
            setUniform(location, var, type, textureIndex);
        } else {
            LOGE("Invalid name");
        }
    }
    void setUniform(int location, const UniformVariant& var, UniformType type, int textureIndex);

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
