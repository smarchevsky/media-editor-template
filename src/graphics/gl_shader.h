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

std::string textFromFile(const std::filesystem::path& path);

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
    Texture2Ddata>

    UniformVariant;

//////////////////////// SHADER //////////////////////////

typedef std::shared_ptr<class GLShader> GLShaderPtr;
class GLShader : NoCopy<GLShader> {
public:
    class Variable {
        int m_location = -1;
        std::string m_name;
        UniformVariant m_data;

    public:
        Variable() = default;
        Variable(
            int location,
            const std::string& m_name,
            UniformVariant m_data)
            : m_location(location)
            , m_name(m_name)
            , m_data(m_data)
        {
        }

        const std::string& getName() const { return m_name; }
        const int getLocation() const { return m_location; }
        const UniformVariant& getData() const { return m_data; }
    };

    GLShader() = default;
    GLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
    GLShader& operator=(GLShader&& rhs);
    ~GLShader();

    static GLShader FromFile(
        const std::filesystem::path& vertRelativePath,
        const std::filesystem::path& fragRelativePath);

    void bind();
    void setUniform(const HashString& name, const UniformVariant& newVar);
    int getHandle() const { return m_shaderProgram; }
    void resetVariables()
    {
        for (const auto& u : m_defaultUniforms) {
            setUniform(u.getLocation(), u.getData());
        }
    }

private:
    int getUniformLocation(const HashString& name) const;
    void setUniform(int location, const UniformVariant& var);

private: // DATA
    const uint32_t m_shaderProgram {};
    const std::unordered_map<HashString, int> m_locations;
    const std::vector<Variable> m_defaultUniforms;

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderManager;
};

#endif // SHADER_H
