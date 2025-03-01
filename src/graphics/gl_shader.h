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

static const std::filesystem::path shaderDir(SHADER_DIR);

struct Texture2Ddata {
    Texture2Ddata() = default;
    Texture2Ddata(const std::shared_ptr<GLTexture2D> texture, int index = -1)
        : m_texture(texture)
        , m_index(index)
    {
    }
    std::shared_ptr<GLTexture2D> m_texture;
    int m_index = -1;
};

typedef std::variant<
    char, // is invalid, dont set char :)

    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4,

    std::vector<float>,
    std::vector<glm::vec2>,
    std::vector<glm::vec3>,
    std::vector<glm::vec4>,
    std::vector<glm::mat4>,

    int,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,

    Texture2Ddata>

    UniformVariant;

typedef std::unordered_map<HashString, UniformVariant> UniformContainer;
//////////////////////// SHADER //////////////////////////

class GLShader : NoCopy<GLShader> {

public:
    static GLShader FromFile(
        const std::filesystem::path& vertRelativePath,
        const std::filesystem::path& fragRelativePath);

    int getHandle() const { return m_shaderProgram; }

    // binds shader and clears uniforms
    void bindAndResetUniforms();

    // set uniforms by objects, if variable was previously marked as Camera type - assert
    void setUniforms(const UniformContainer& newUniforms);

    // set camera uniforms right after bind reset uniforms
    void setCameraUniforms(const UniformContainer& cameraUniforms);

    GLShader() = default;
    GLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
    ~GLShader();

    // clang-format off
    enum class UniformType : uint8_t { Default, Camera };
    enum class UniformStatus : uint8_t { DontTouch, MustUpdate, MustResetToDefault };

    class Variable {
        UniformVariant m_defaultData;
        mutable UniformVariant m_currentData;
        int m_location = -1;
        std::string m_name;
        mutable UniformType m_type = UniformType::Default;
        mutable UniformStatus m_status = UniformStatus::MustUpdate; // variables are dirty by default, to update in shader state machine

    public:
        Variable() = default;
        Variable(int location, const std::string& m_name, UniformVariant data)
            : m_defaultData(data), m_currentData(data),  m_location(location), m_name(m_name)  {}

        friend class GLShader;
        friend class ShaderCodeParser;
    };
    // clang-format on
private:
    UniformType getCurrentUniformType(int location) const { return m_uniforms[location].m_type; }

    void setUniformInternal(int location, const UniformVariant& var);

public:
    GLShader& operator=(GLShader&& rhs)
    {
        const_cast<uint32_t&>(m_shaderProgram) = rhs.m_shaderProgram;
        const_cast<uint32_t&>(rhs.m_shaderProgram) = 0;

        const_cast<std::unordered_map<HashString, int>&>(m_locations) = std::move(rhs.m_locations);
        const_cast<std::vector<Variable>&>(m_uniforms) = std::move(rhs.m_uniforms);
        return *this;
    }

private: // DATA
    const uint32_t m_shaderProgram {};
    const std::unordered_map<HashString, int> m_locations;

    const std::vector<Variable> m_uniforms;

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderManager;
};

#endif // SHADER_H
