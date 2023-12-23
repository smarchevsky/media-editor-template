#include "gl_shader.h"

#include "common.h"
// #include "shadercodeparser.h"

#define ERRORCHEKING
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

uint32_t GLShader::s_currentBindedShaderHandle = 0;

namespace {

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

UniformVariant createDefaultUniformData(int GLtype, int size, int textureIndex)
{
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

std::vector<GLShader::Variable> getUniformList(int program)
{
    GLint varCount;
    GLint varSize; // size of the variable
    GLenum varType; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei varBufSize = 512; // maximum name length
    GLchar varName[varBufSize] {}; // variable name in GLSL
    GLsizei varNameLength; // name length

    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &varCount);
    printf("Active Attributes: %d\n", varCount);
    for (int i = 0; i < varCount; i++) {
        glGetActiveAttrib(program, (GLuint)i, varBufSize, &varNameLength, &varSize, &varType, varName);
        printf("  - Attribute #%d Type: %u Name: %s\n", i, varType, varName);
    }

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &varCount);
    printf("Active Uniforms: %d\n", varCount);

    int texture2DIndex = 0;
    std::vector<GLShader::Variable> uniformVariables;
    for (int i = 0; i < varCount; i++) {
        glGetActiveUniform(program, (GLuint)i, varBufSize, &varNameLength, &varSize, &varType, varName);

        const std::string nameString(varName);
        GLShader::Variable var(i, nameString, createDefaultUniformData(varType, varSize, texture2DIndex));
        uniformVariables.push_back(std::move(var));

        printf("  - Uniform #%d, Size: %d, Type: %u, Name: %s\n", i, varSize, varType, varName);

        if (varType == GL_SAMPLER_2D)
            texture2DIndex++;
    }
    printf("\n");
    return uniformVariables;
}
} // namespace

GLShader::GLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode)
{
    GLint success = GL_FALSE;
    if (vertexShaderCode.size() && fragmentShaderCode.size()) {
        UNCONST(m_shaderProgram) = glCreateProgram();
        GLuint vs = createShader(vertexShaderCode.c_str(), GL_VERTEX_SHADER);
        GLuint fs = createShader(fragmentShaderCode.c_str(), GL_FRAGMENT_SHADER);
        glAttachShader(m_shaderProgram, vs);
        glAttachShader(m_shaderProgram, fs);
        glLinkProgram(m_shaderProgram);

        char infoLog[512];
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

        if (success == GL_TRUE) {
            std::vector<Variable> uniforms = getUniformList(getHandle());

            // ShaderCodeParser::parseDefaultUniforms(vertexShaderCode, uniforms);
            // ShaderCodeParser::parseDefaultUniforms(fragmentShaderCode, uniforms);

            UNCONST(m_uniforms) = std::move(uniforms);
            for (const auto& d : m_uniforms)
                UNCONST(m_locations).insert({ d.m_name.c_str(), d.m_location });

        } else {

            glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
            std::cerr << "Linking failed: " << infoLog << std::endl;
            UNCONST(m_shaderProgram) = 0;

            assert(false && "Shader compilation failed");
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
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

// int GLShader::getUniformLocation(const HashString& name) const { return glGetUniformLocation(m_shaderProgram, name.getString().c_str()); }

GLShader::~GLShader()
{
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        LOG("Shader destroyed");
    }
}

void GLShader::setUniformInternal(int location, const UniformVariant& uniformVariable)
{
    const auto& currentDefaultUniform = m_uniforms[location].m_defaultData;
    assert(uniformVariable.index() == currentDefaultUniform.index() && "Variable must match shader type");
    switch (uniformVariable.index()) {

    case GET_INDEX(Texture2Ddata): {
        const auto& var = std::get<Texture2Ddata>(uniformVariable);

        const auto& sharedTexture = var.m_texture;

        int textureIndex = 0;
        int textureHandle = 0;

        if (sharedTexture) {
            textureIndex = var.m_index;
            textureHandle = sharedTexture->getHandle();
            sharedTexture->generateMipMapsIfDirty();
        }

        if (var.m_index < 0) // reset to default texture index in shader
            textureIndex = std::get<Texture2Ddata>(currentDefaultUniform).m_index;

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

void GLShader::setCameraUniforms(const UniformContainer& cameraUniforms)
{
    for (const auto& u : cameraUniforms) {
        const auto& cameraUniformName = u.first;
        const auto& cameraUniformVariable = u.second;
        const auto varLocationIter = m_locations.find(cameraUniformName);

        if (varLocationIter != m_locations.end()) {
            int varLocation = varLocationIter->second;

            // mark as camera type
            m_uniforms[varLocation].m_currentData = cameraUniformVariable;
            m_uniforms[varLocation].m_type = UniformType::Camera;
            m_uniforms[varLocation].m_status = UniformStatus::MustUpdate;
        }
    }
}

void GLShader::setUniforms(const UniformContainer& newUniforms)
{
    for (const auto& u : newUniforms) {
        const auto& newUniformName = u.first;
        const auto& newUniformVariable = u.second;
        const auto varLocationIter = m_locations.find(newUniformName);

        if (varLocationIter != m_locations.end()) {
            int varLocation = varLocationIter->second;

            assert(getCurrentUniformType(varLocation) == UniformType::Default && "Uniform was previously marked as Camera type.");

            m_uniforms[varLocation].m_currentData = newUniformVariable;
            m_uniforms[varLocation].m_status = UniformStatus::MustUpdate;
        }
    }

    // state machine magic
    for (auto& u : m_uniforms) {

        // camera updates only once after bind, and remain unchanged untill next bind
        if (u.m_type == UniformType::Camera) {
            if (u.m_status == UniformStatus::MustUpdate) {
                setUniformInternal(u.m_location, u.m_currentData);
                u.m_status = UniformStatus::DontTouch;

            } else if (u.m_status == UniformStatus::MustResetToDefault) {
                setUniformInternal(u.m_location, u.m_defaultData);
                u.m_status = UniformStatus::DontTouch;
            }
            continue;
        }

        // if variable is updated - update it in shader
        if (u.m_status == UniformStatus::MustUpdate) {
            setUniformInternal(u.m_location, u.m_currentData);
            u.m_currentData = u.m_defaultData;
            u.m_status = UniformStatus::MustResetToDefault;

            // if variable was previously updated - reset it to default
        } else if (u.m_status == UniformStatus::MustResetToDefault) {
            setUniformInternal(u.m_location, u.m_defaultData);
            u.m_status = UniformStatus::DontTouch;
        }
    }
}

void GLShader::bindAndResetUniforms()
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

    // all default uniforms are dirty, don't change it
    // so, they will be overwritten in
    for (auto& u : m_uniforms) {
        u.m_status = UniformStatus::MustResetToDefault;
    }
}
