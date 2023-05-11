#include "gl_shader.h"
#include "helper_general.h"

#define ERRORCHEKING
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

static const std::filesystem::path shaderDir(SHADER_DIR);
uint32_t GLShader::s_currentBindedShaderHandle = 0;

namespace {
/// https://gist.github.com/nnaumenko/1db96f7e187979a057ee7ad757dee4f2
/// @return Index of type T in variant V; or variant size if V does not include an alternative for T.
template <typename V, typename T, size_t I = 0>
constexpr size_t variant_index()
{
    if constexpr (I >= std::variant_size_v<V>) {
        return (std::variant_size_v<V>);
    } else {
        if constexpr (std::is_same_v<std::variant_alternative_t<I, V>, T>) {
            return (I);
        } else {
            return (variant_index<V, T, I + 1>());
        }
    }
}

UniformVariant createDefaultUniformData(int GLtype, int size, int textureIndex)
{
    // constexpr int floatSize = sizeof(float);
    // constexpr int intSize = sizeof(int);

    switch (GLtype) {
    case GL_FLOAT:
        return 0.f;

    case GL_FLOAT_VEC2:
        return glm::vec2(0);

    case GL_FLOAT_VEC3:
        return glm::vec3(0);

    case GL_FLOAT_VEC4:
        return glm::vec4(0);

    case GL_FLOAT_MAT4:
        return glm::mat4(1);

    case GL_INT:
        return int(0);

    case GL_INT_VEC2:
        return glm::ivec2(0);

    case GL_INT_VEC3:
        return glm::ivec3(0);

    case GL_INT_VEC4:
        return glm::ivec4(0);

    case GL_SAMPLER_2D: {
        // assert(size == intSize);
        return Texture2Ddata(nullptr, textureIndex);
    }
    }

    assert(false && "Unsupported uniform format");
    return {};
}

std::vector<GLShader::Variable> getUniformList(GLShader* shader)
{
    const auto program = shader->getHandle();

    GLint count;
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 512; // maximum name length
    GLchar name[bufSize] {}; // variable name in GLSL
    GLsizei length; // name length

    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
    printf("Active Attributes: %d\n", count);
    for (int i = 0; i < count; i++) {
        glGetActiveAttrib(program, (GLuint)i, bufSize, &length, &size, &type, name);
        printf("  - Attribute #%d Type: %u Name: %s\n", i, type, name);
    }

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);

    std::vector<GLShader::Variable> result;

    int texture2DIndex = 0;
    for (int i = 0; i < count; i++) {
        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
        std::string nameStr(name);

        GLShader::Variable var(i, nameStr,
            createDefaultUniformData(type, size, texture2DIndex));

        result.push_back(std::move(var));

        printf("  - Uniform #%d Type: %u Name: %s\n", i, type, name);

        if (type == GL_SAMPLER_2D)
            texture2DIndex++;
    }
    printf("\n");
    return result;
}

int createShader(const char* shaderSource, int shaderType)
{
    const char* shaderTypeName {};
    switch (shaderType) {
    case GL_VERTEX_SHADER: {
        shaderTypeName = "Vertex";
    } break;
    case GL_FRAGMENT_SHADER: {
        shaderTypeName = "Fragment";
    } break;
    default: {
        std::cerr << "Invalid shader type" << std::endl;
        return -1;
    }
    }

    unsigned int currentShader = glCreateShader(shaderType);
    glShaderSource(currentShader, 1, &shaderSource, NULL);
    glCompileShader(currentShader);

    int success;
    char infolog[512];
    glGetShaderiv(currentShader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        // (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
        int newSize;
        glGetShaderInfoLog(currentShader, 512, &newSize, infolog);
        std::cerr << shaderTypeName << " shader compilation failed" << infolog << std::endl;
    }

    return currentShader;
}

} // namespace

GLShader::GLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode)
{
    GLint success = GL_FALSE;
    if (vertexShaderCode.size() && fragmentShaderCode.size()) {
        const_cast<uint32_t&>(m_shaderProgram) = glCreateProgram();
        GLuint vs = createShader(vertexShaderCode.c_str(), GL_VERTEX_SHADER);
        GLuint fs = createShader(fragmentShaderCode.c_str(), GL_FRAGMENT_SHADER);
        glAttachShader(m_shaderProgram, vs);
        glAttachShader(m_shaderProgram, fs);
        glLinkProgram(m_shaderProgram);

        char infoLog[512];
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

        if (success == GL_TRUE) {
            const_cast<std::vector<Variable>&>(m_defaultUniforms) = getUniformList(this);
            auto& locations = const_cast<std::unordered_map<HashString, int>&>(m_locations);
            for (const auto& d : m_defaultUniforms) {
                locations.insert({ d.getName().c_str(), d.getLocation() });
            }

        } else {

            glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
            std::cerr << "Linking failed: " << infoLog << std::endl;
            const_cast<uint32_t&>(m_shaderProgram) = 0;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
}

GLShader& GLShader::operator=(GLShader&& rhs)
{
    const_cast<uint32_t&>(m_shaderProgram) = rhs.m_shaderProgram;
    const_cast<uint32_t&>(rhs.m_shaderProgram) = 0;
    const_cast<std::unordered_map<HashString, int>&>(m_locations) = std::move(rhs.m_locations);
    const_cast<std::vector<Variable>&>(m_defaultUniforms) = std::move(rhs.m_defaultUniforms);
    return *this;
}

GLShader GLShader::FromFile(
    const std::filesystem::path& vertRelativePath,
    const std::filesystem::path& fragRelativePath)
{
    printf("Loading shader: %s, %s\n", vertRelativePath.c_str(), fragRelativePath.c_str());
    return GLShader(
        textFromFile(shaderDir / vertRelativePath),
        textFromFile(shaderDir / fragRelativePath));
}

//int GLShader::getUniformLocation(const HashString& name) const
//{
//    return glGetUniformLocation(m_shaderProgram, name.getString().c_str());
//}

GLShader::~GLShader()
{
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        LOG("Shader destroyed");
    }
}

void GLShader::setUniform(const HashString& name, const UniformVariant& newVar)
{
    auto it = m_locations.find(name);
    if (it != m_locations.end()) {
        setUniform(it->second, newVar);
    } else {
        // LOGE("Variable: " << name.getString() << " did not find in shader.");
    }
}

#define GET_INDEX(type) variant_index<UniformVariant, type>()
void GLShader::setUniform(int location, const UniformVariant& uniformVariable)
{
    const auto& currentDefaultUniform = m_defaultUniforms[location].getData();
    assert(uniformVariable.index() == currentDefaultUniform.index() && "Variable must match shader type");
    switch (uniformVariable.index()) {

    case GET_INDEX(Texture2Ddata): {
        const auto& var = std::get<Texture2Ddata>(uniformVariable);

        const auto& sharedTexture = var.m_texture;

        int textureIndex = var.m_index;
        if (var.m_index < 0) {
            const auto& defaultVar = std::get<Texture2Ddata>(currentDefaultUniform);
            textureIndex = defaultVar.m_index;
        }

        textureIndex = sharedTexture ? textureIndex : 0;

        int textureHandle = sharedTexture ? sharedTexture->getHandle() : 0;

        if (textureIndex < 0 || textureIndex >= 32) {
            LOGE("Invalid texture index: " << textureIndex);
            textureIndex = 0;
        }

        glBindTextureUnit(textureIndex, textureHandle);
        glUniform1i(location, textureIndex);

// #define VERBOSE_LOG
#ifdef VERBOSE_LOG
        LOG("Texture set: location: " << location << ", handle: "
                                      << textureHandle << ", index: "
                                      << (int)textureIndex);
#endif
    } break;

    case GET_INDEX(float): {
        const auto& var = std::get<float>(uniformVariable);
        glUniform1f(location, var);
    } break;

    case GET_INDEX(glm::vec2): {
        const auto& var = std::get<glm::vec2>(uniformVariable);
        glUniform2fv(location, 1, &var[0]);
    } break;

    case GET_INDEX(glm::vec3): {
        const auto& var = std::get<glm::vec3>(uniformVariable);
        glUniform3fv(location, 1, &var[0]);
    } break;

    case GET_INDEX(glm::vec4): {
        const auto& var = std::get<glm::vec4>(uniformVariable);
        glUniform4fv(location, 1, &var[0]);
    } break;

    case GET_INDEX(glm::mat4): {
        const auto& var = std::get<glm::mat4>(uniformVariable);
        glUniformMatrix4fv(location, 1, GL_FALSE, &var[0][0]);
    } break;

    case GET_INDEX(int): {
        const auto& var = std::get<int>(uniformVariable);
        glUniform1i(location, var);
    } break;

    case GET_INDEX(glm::ivec2): {
        const auto& var = std::get<glm::ivec2>(uniformVariable);
        glUniform2iv(location, 1, &var[0]);
    } break;

    case GET_INDEX(glm::ivec3): {
        const auto& var = std::get<glm::ivec3>(uniformVariable);
        glUniform3iv(location, 1, &var[0]);
    } break;

    case GET_INDEX(glm::ivec4): {
        const auto& var = std::get<glm::ivec4>(uniformVariable);
        glUniform4iv(location, 1, &var[0]);
    } break;

    default: {
        LOGE("Unsupported uniform variable type");
    }
    }
}

void GLShader::bind()
{
    // #define ALWAYS_BIND
#ifdef ALWAYS_BIND
    glUseProgram(m_shaderProgram);
#else
    if (s_currentBindedShaderHandle != m_shaderProgram) {
        s_currentBindedShaderHandle = m_shaderProgram;
        glUseProgram(m_shaderProgram);
    }
#endif
}

std::string textFromFile(const std::filesystem::path& path)
{
    std::string sourceCode;
    std::ifstream codeStream(path, std::ios::in);
    if (codeStream.is_open()) {
        std::stringstream sstr;
        sstr << codeStream.rdbuf();
        sourceCode = sstr.str();
        codeStream.close();
    } else {
        LOGE("Can't open file: " << path);
    }
    return sourceCode;
}
