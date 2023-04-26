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

typedef std::tuple<std::shared_ptr<GLTexture>, int> Texture2Ddata;

typedef std::variant<
    char, // is invalid, dont set char :)
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4,
    Texture2Ddata>

    UniformVariant;

enum class UniformDependency : uint8_t {
    Object, // object-related variables
    View, // view-related variables
    Free // non-related shader-wise variables
};

//////////////////////// SHADER //////////////////////////

typedef std::shared_ptr<class GLShader> GLShaderPtr;
class GLShader : NoCopy<GLShader> {
public:
    class Variable {
        GLShader* m_shader;
        int m_location = -1;
        std::string m_name;
        UniformDependency m_dependency;
        UniformVariant m_data;

    public:
        const std::string& getName() const { return m_name; }
        const int getLocation() const { return m_location; }
        UniformDependency getDependency() const { return m_dependency; }

        Variable() = default;
        Variable(
            int location,
            const std::string& m_name,
            UniformDependency m_dependency,
            UniformVariant m_data)
            : m_location(location)
            , m_name(m_name)
            , m_dependency(m_dependency)
            , m_data(m_data)
        {
        }
        const UniformVariant& getData() const { return m_data; }
        void setData(const UniformVariant& data);
        void setData(const std::shared_ptr<GLTexture>& newTexture);
        bool isValid() { return (m_location != -1); }
    };

    typedef std::unordered_map<HashString, Variable> UniformVariables;

    //////////////////////// VARIABLE LIST //////////////////////////

    class Instance {
        GLShaderPtr m_shader;

    public:
        Instance() = default;
        Instance(const GLShaderPtr& shader, UniformDependency dependency)
            : m_shader(shader)
        {
            assert(m_shader);
            for (const auto& u : shader->getUniforms()) {
                if (u.second.getDependency() == dependency) {
                    m_variables.insert(u);
                }
            }
        }

        template <class T>
        void set(const HashString& name, const T& data)
        {
            auto it = m_variables.find(name);
            if (it != m_variables.end()) {
                it->second.setData(data);
            } else {
                LOGE("You are trying to set data with unexisted name: " << name.getString());
            }
        }

        void applyUniforms(bool forceBindShader = true)
        {
            if (m_shader) {
                if (forceBindShader)
                    m_shader->bind();
                for (const auto& v : m_variables) {
                    m_shader->setUniform(v.second.getLocation(), v.second.getData());
                }
            }
        }
        GLShader* getShader() { return m_shader.get(); }
        UniformVariables m_variables;
    };

    GLShader(const char* vertexShaderCode, const char* fragmentShaderCode);
    ~GLShader();

    void bind();
    void setUniform(const HashString& name, const UniformVariant& var);

    int getHandle() const { return m_shaderProgram; }
    bool valid() const { return m_shaderProgram != 0; }

    void resetVariables(UniformDependency dependency)
    {
        for (const auto& vPair : m_defaultVariables) {
            const auto& v = vPair.second;
            if (v.getDependency() == dependency)
                setUniform(v.getLocation(), v.getData());
        }
    }
    const UniformVariables& getUniforms() const { return m_defaultVariables; }

private:
    int getUniformLocation(const HashString& name) const;
    void setUniform(int location, const UniformVariant& var);

private: // DATA
    uint32_t m_shaderProgram {};

    const UniformVariables m_defaultVariables;

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderManager;
};

//////////////////////// SHADER MANAGER //////////////////////////

class GLShaderManager {

public:
    static GLShaderManager& get()
    {
        static GLShaderManager shaderGeneratorStatic;
        return shaderGeneratorStatic;
    }
    GLShaderPtr addShader(
        const char* uniqueName,
        const char* vertexShaderCode,
        const char* fragmentShaderCode);

    GLShaderPtr addShader(
        const char* uniqueName,
        const fs::path& vertexShaderPath,
        const fs::path& fragmentShaderPath);

    GLShaderPtr getDefaultShader2d();
    GLShaderPtr getByName(const HashString& name);

private:
    GLShaderManager() = default;
    ~GLShaderManager() = default;

    static std::unordered_map<HashString, GLShaderPtr> s_staticShaders;
};
inline std::unordered_map<HashString, GLShaderPtr> GLShaderManager::s_staticShaders;

#endif // SHADER_H
