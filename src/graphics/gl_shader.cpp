#include "gl_shader.h"

#include "common.h"
// #include "shadercodeparser.h"

#define ERRORCHEKING
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <cstring>
#include <iostream>

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

UniformVariant createDefaultUniformData(int GLtype, int size, int textureIndex, bool isArray)
{
    if (isArray /*size > 1*/) {
        switch (GLtype) {

        case GL_FLOAT:
            return std::vector<float>(0);
        case GL_FLOAT_VEC2:
            return std::vector<glm::vec2>(0);
        case GL_FLOAT_VEC3:
            return std::vector<glm::vec3>(0);
        case GL_FLOAT_VEC4:
            return std::vector<glm::vec4>(0);
        case GL_FLOAT_MAT4:
            return std::vector<glm::mat4>(0);
        }

    } else {
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
    }

    assert(false && "Unsupported uniform format");
    return {};
}

std::unordered_map<int, GLShader::Variable> getUniformList(int program)
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
        printf("  - Attribute #%d Type: %u, Size: %u, Name: %s\n", i, varType, varSize, varName);
    }

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &varCount);
    printf("Active Uniforms: %d\n", varCount);

    int texture2DIndex = 0;
    std::unordered_map<int, GLShader::Variable> uniformVariables;
    for (int i = 0; i < varCount; i++) {
        glGetActiveUniform(program, (GLuint)i, varBufSize, &varNameLength, &varSize, &varType, varName);

        std::string nameString(varName);
        auto openBracketIndex = nameString.find('[');
        bool isArray = openBracketIndex != -1;
        if (isArray)
            nameString = nameString.substr(0, openBracketIndex);

        int location = glGetUniformLocation(program, nameString.c_str());

        GLShader::Variable var(location, nameString, createDefaultUniformData(varType, varSize, texture2DIndex, isArray));
        uniformVariables[location] = std::move(var);

        printf("  - Uniform #%d, Size: %d, Type: %u, Name: %s, rawName: %s\n",
            location, varSize, varType, nameString.c_str(), varName);

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
            std::unordered_map<int, Variable> uniforms = getUniformList(m_shaderProgram);
            // volatile int dataLocation = glGetUniformLocation(m_shaderProgram, "data");
            // volatile int timeLocation = glGetUniformLocation(m_shaderProgram, "time");

            // ShaderCodeParser::parseDefaultUniforms(vertexShaderCode, uniforms);
            // ShaderCodeParser::parseDefaultUniforms(fragmentShaderCode, uniforms);

            UNCONST(m_uniforms) = std::move(uniforms);
            for (const auto& pair : m_uniforms) {
                auto& uniformData = pair.second;
                UNCONST(m_locations).insert({ uniformData.m_name.c_str(), uniformData.m_location });
            }

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

// #define ENABLE_UNIFORMS_LOGGING
#ifdef ENABLE_UNIFORMS_LOGGING
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& arr)
{
    os << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int>) {
            os << arr[i];
            // } else if constexpr (std::is_same_v<T, glm::vec2>) {
        } else {
            os << glm::to_string(arr[i]);
        }
        if (i < arr.size() - 1)
            os << ", ";
    }
    os << "]";
    return os;
}
#define LOG_UNIFORMS(x) x;
#else
#define LOG_UNIFORMS(x)
#endif

void GLShader::setUniformInternal(int location, const UniformVariant& uniformVariable)
{

    auto uniformIt = m_uniforms.find(location);
    if (uniformIt == m_uniforms.end())
        return;
    const auto& uniform = uniformIt->second;

    const auto& currentDefaultUniform = uniform.m_defaultData;
    assert(uniformVariable.index() == currentDefaultUniform.index() && "Variable must match shader type");

    LOG_UNIFORMS(std::cout << "Name: " << uniform.m_name << ",\t Variant index: " << uniformVariable.index() << ", type: ")

    switch (uniformVariable.index()) {

    case GET_UNIFORM_VARIANT_INDEX(Texture2Ddata): {
        const auto& var = std::get<Texture2Ddata>(uniformVariable);
        LOG_UNIFORMS(std::cout << "Texture2Ddata")
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

    case GET_UNIFORM_VARIANT_INDEX(float): {
        const auto& var = std::get<float>(uniformVariable);
        LOG_UNIFORMS(std::cout << "float: " << var)
        glUniform1f(location, var);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::vec2): {
        const auto& var = std::get<glm::vec2>(uniformVariable);
        LOG_UNIFORMS(std::cout << "vec2: " << glm::to_string(var))
        glUniform2fv(location, 1, &var[0]);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::vec3): {
        const auto& var = std::get<glm::vec3>(uniformVariable);
        LOG_UNIFORMS(std::cout << "vec3: " << glm::to_string(var))
        glUniform3fv(location, 1, &var[0]);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::vec4): {
        const auto& var = std::get<glm::vec4>(uniformVariable);
        LOG_UNIFORMS(std::cout << "vec4: " << glm::to_string(var))
        glUniform4fv(location, 1, &var[0]);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::mat4): {
        const auto& var = std::get<glm::mat4>(uniformVariable);
        LOG_UNIFORMS(std::cout << "mat4: " << glm::to_string(var))
        glUniformMatrix4fv(location, 1, GL_FALSE, &var[0][0]);
    } break;

        // VECTORS

    case GET_UNIFORM_VARIANT_INDEX(std::vector<float>): {
        const auto& var = std::get<std::vector<float>>(uniformVariable);
        LOG_UNIFORMS(std::cout << "float vector: " << var)
        glUniform1fv(location, var.size(), var.data());
    } break;

    case GET_UNIFORM_VARIANT_INDEX(std::vector<glm::vec2>): {
        const auto& var = std::get<std::vector<glm::vec2>>(uniformVariable);
        LOG_UNIFORMS(std::cout << "vec2 vector: " << var)
        glUniform2fv(location, var.size(), &var[0].x);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(std::vector<glm::vec3>): {
        const auto& var = std::get<std::vector<glm::vec3>>(uniformVariable);
        LOG_UNIFORMS(std::cout << "vec3 vector: " << var)
        glUniform3fv(location, var.size(), &var[0].x);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(std::vector<glm::vec4>): {
        const auto& var = std::get<std::vector<glm::vec4>>(uniformVariable);
        LOG_UNIFORMS(std::cout << "vec4 vector: " << var)
        glUniform4fv(location, var.size(), &var[0].x);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(std::vector<glm::mat4>): {
        const auto& var = std::get<std::vector<glm::mat4>>(uniformVariable);
        LOG_UNIFORMS(std::cout << "mat4: " << var)
        glUniformMatrix4fv(location, var.size(), GL_FALSE, &var[0][0].x);
    } break;

        // VECTORS

    case GET_UNIFORM_VARIANT_INDEX(int): {
        const auto& var = std::get<int>(uniformVariable);
        LOG_UNIFORMS(std::cout << "int: " << var)
        glUniform1i(location, var);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::ivec2): {
        const auto& var = std::get<glm::ivec2>(uniformVariable);
        LOG_UNIFORMS(std::cout << "ivec2: " << glm::to_string(var))
        glUniform2iv(location, 1, &var[0]);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::ivec3): {
        const auto& var = std::get<glm::ivec3>(uniformVariable);
        LOG_UNIFORMS(std::cout << "ivec3: " << glm::to_string(var))
        glUniform3iv(location, 1, &var[0]);
    } break;

    case GET_UNIFORM_VARIANT_INDEX(glm::ivec4): {
        const auto& var = std::get<glm::ivec4>(uniformVariable);
        LOG_UNIFORMS(std::cout << "ivec4: " << glm::to_string(var))
        glUniform4iv(location, 1, &var[0]);
    } break;

    default: {
        LOGE("Unsupported uniform variable type");
    }
    }
#ifdef ENABLE_UNIFORMS_LOGGING
    std::cout << std::endl;
#endif
}

void GLShader::setCameraUniforms(const UniformContainer& cameraUniforms)
{
    for (const auto& u : cameraUniforms) {
        const auto& cameraUniformName = u.first;
        const auto& cameraUniformVariable = u.second;
        const auto varLocationIter = m_locations.find(cameraUniformName);

        if (varLocationIter != m_locations.end()) {
            int varLocation = varLocationIter->second;

            auto uniformIt = m_uniforms.find(varLocation);
            if (uniformIt == m_uniforms.end())
                continue;
            auto& uniform = uniformIt->second;
            // mark as camera type
            uniform.m_currentData = cameraUniformVariable;
            uniform.m_type = UniformType::Camera;
            uniform.m_status = UniformStatus::MustUpdate;
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

            auto uniformIt = m_uniforms.find(varLocation);
            if (uniformIt == m_uniforms.end())
                continue;
            auto& uniform = uniformIt->second;

            assert(uniform.m_type == UniformType::Default && "Uniform was previously marked as Camera type.");

            uniform.m_currentData = newUniformVariable;
            uniform.m_status = UniformStatus::MustUpdate;
        }
    }

    // state machine magic
    for (auto& pair : m_uniforms) {
        auto& uniform = pair.second;

        // camera updates only once after bind, and remain unchanged untill next bind
        if (uniform.m_type == UniformType::Camera) {
            if (uniform.m_status == UniformStatus::MustUpdate) {
                setUniformInternal(uniform.m_location, uniform.m_currentData);
                uniform.m_status = UniformStatus::DontTouch;

            } else if (uniform.m_status == UniformStatus::MustResetToDefault) {
                setUniformInternal(uniform.m_location, uniform.m_defaultData);
                uniform.m_status = UniformStatus::DontTouch;
            }
            continue;
        }

        // if variable is updated - update it in shader
        if (uniform.m_status == UniformStatus::MustUpdate) {
            setUniformInternal(uniform.m_location, uniform.m_currentData);
            uniform.m_currentData = uniform.m_defaultData;
            uniform.m_status = UniformStatus::MustResetToDefault;

            // if variable was previously updated - reset it to default
        } else if (uniform.m_status == UniformStatus::MustResetToDefault) {
            setUniformInternal(uniform.m_location, uniform.m_defaultData);
            uniform.m_status = UniformStatus::DontTouch;
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
    for (auto& pair : m_uniforms) {
        auto& uniform = pair.second;
        uniform.m_status = UniformStatus::MustResetToDefault;
    }
}
