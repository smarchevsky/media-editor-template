#include "gl_shader.h"
#include "gl_shadersources.h"
#include "helper_general.h"

#define ERRORCHEKING
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

namespace {
UniformType getUniformTypeFromGLType(int GLtype, int size)
{
    constexpr int floatSize = sizeof(float);
    constexpr int intSize = sizeof(int);

    switch (GLtype) {
    case GL_FLOAT: {
        // assert(size == floatSize);
        return UniformType::Float;
    };

    case GL_FLOAT_VEC2: {
        // assert(size == floatSize * 2);
        return UniformType::Vec2;
    }

    case GL_FLOAT_VEC3: {
        // assert(size == floatSize * 3);
        return UniformType::Vec3;
    };

    case GL_FLOAT_VEC4: {
        // assert(size == floatSize * 4);
        return UniformType::Vec4;
    };

    case GL_SAMPLER_2D: {
        // assert(size == intSize);
        return UniformType::Texture2D;
    }

    case GL_FLOAT_MAT4: {
        // assert(size == floatSize * 16);
        return UniformType::Mat4;
    };
    }

    assert(false && "Unsupported uniform format");
    return {};
}

UniformList getUniformList(GLuint program)
{
    UniformList result;
    GLint count;
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 512; // maximum name length
    GLchar name[bufSize] {}; // variable name in GLSL
    GLsizei length; // name length

    /*
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
    printf("Active Attributes: %d\n", count);
    for (int i = 0; i < count; i++)
    {
        glGetActiveAttrib(program, (GLuint)i, bufSize, &length, &size, &type, name);
        printf("Attribute #%d Type: %u Name: %s\n", i, type, name);
    } */

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);

    int textureIndex = 0;
    for (int i = 0; i < count; i++) {
        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);

        UniformInfo info;
        info.name = name;
        info.type = getUniformTypeFromGLType(type, size);
        info.textureIndex = textureIndex;

        result.push_back(info);

        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);

        if (info.type == UniformType::Texture2D)
            textureIndex++;
    }
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

bool createShaderProgram(const char* vertexShaderCode, const char* fragmentShaderCode,
    uint32_t& outShaderProgram, UniformList& outUniforms)
{
    GLint success = GL_FALSE;
    if (vertexShaderCode && fragmentShaderCode) {
        GLuint shaderProgramHandle = glCreateProgram();
        GLuint vs = createShader(vertexShaderCode, GL_VERTEX_SHADER);
        GLuint fs = createShader(fragmentShaderCode, GL_FRAGMENT_SHADER);
        glAttachShader(shaderProgramHandle, vs);
        glAttachShader(shaderProgramHandle, fs);
        glLinkProgram(shaderProgramHandle);

        char infoLog[512];
        glGetProgramiv(shaderProgramHandle, GL_LINK_STATUS, &success);
        if (success == GL_TRUE) {
            outShaderProgram = shaderProgramHandle;
            outUniforms = getUniformList(shaderProgramHandle);
        } else {
            glGetProgramInfoLog(shaderProgramHandle, 512, NULL, infoLog);
            std::cerr << "Linking failed: " << infoLog << std::endl;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    return success == GL_TRUE;
}
} // namespace

uint32_t GLShader::s_currentBindedShaderHandle = 0;

int GLShader::getUniformLocation(const char* name)
{
    return glGetUniformLocation(m_shaderProgram, name);
}

GLShader::~GLShader()
{
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        LOG("Shader destroyed");
    }
}

GLShader::GLShader(GLShader&& r)
{
    m_shaderProgram = r.m_shaderProgram;
    m_uniforms = std::move(r.m_uniforms);
    r.m_shaderProgram = 0;
}

void GLShader::setUniform(int location, const UniformVariant& uniformVariable,
    UniformType type, int textureIndex)
{
    switch (type) {

    case UniformType::Texture2D: {
        const auto& var = std::get_if<std::shared_ptr<GLTexture>>(&uniformVariable);
        GLTexture* texture = var ? var->get() : nullptr;

        if (textureIndex < 0 || textureIndex >= 32) {
            LOGE("Invalid texture index: " << textureIndex);
            textureIndex = 0;
        }

        int textureHandle = var ? texture->getHandle() : 0;
        textureIndex = var ? textureIndex : 0;

        glBindTextureUnit(textureIndex, textureHandle);
        glUniform1i(location, textureIndex);

#ifdef PRECISE_LOG
        if (!var)
            LOGE("Texture var not set");
        else
            LOG("Texture set: location: " << location << ", handle: " << textureHandle << ", index: " << (int)textureIndex);
#endif
    } break;

    case UniformType::Float: {
        const auto& var = std::get_if<float>(&uniformVariable);
        glUniform1f(location, var ? *var : 0.f);
#ifdef PRECISE_LOG
        if (!var)
            LOGE("Float var not set");
#endif
    } break;

    case UniformType::Vec2: {
        const auto* var = std::get_if<glm::vec2>(&uniformVariable);
        glm::vec2 result = var ? *var : glm::vec2(0);
        glUniform2fv(location, 1, &result[0]);
#ifdef PRECISE_LOG
        if (!var)
            LOGE("Vec2 var not set");
#endif
    } break;

    case UniformType::Vec3: {
        const auto& var = std::get_if<glm::vec3>(&uniformVariable);
        glm::vec2 result = var ? *var : glm::vec3(0);
        glUniform3fv(location, 1, &result[0]);
#ifdef PRECISE_LOG
        if (!var)
            LOGE("Vec3 var not set");
#endif
    } break;

    case UniformType::Vec4: {
        const auto& var = std::get_if<glm::vec4>(&uniformVariable);
        glm::vec4 result = var ? *var : glm::vec4(0);
        glUniform4fv(location, 1, &result[0]);
#ifdef PRECISE_LOG
        if (!var)
            LOGE("Vec4 var not set");
#endif
    } break;

    case UniformType::Mat4: {
        const auto& var = std::get_if<glm::mat4>(&uniformVariable);
        glm::mat4 result = var ? *var : glm::mat4(1);
        glUniformMatrix4fv(location, 1, GL_FALSE, &result[0][0]);
#ifdef PRECISE_LOG
        if (!var)
            LOGE("Mat4 var not set");
        else
            LOG("Mat4 set location: " << location);
#endif
    } break;

    default: {
        LOGE("Unsupported uniform variable type");
    }
    }
}

void GLShaderInstance::updateUniform(HashString name, const UniformVariant& var)
{
    auto it = m_savedUniforms.find(name);
    if (it != m_savedUniforms.end()) {
        UniformData& data = it->second;
        data.dataVariant = var;
    }
}

UniformVariant GLShaderInstance::getUniform(HashString name)
{
    auto it = m_savedUniforms.find(name);
    if (it != m_savedUniforms.end()) {
        UniformData& data = it->second;
        return data.dataVariant;
    }
    return {};
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

//////////////////////// SHADER MANAGER //////////////////////////

std::unordered_map<HashString, GLShader> GLShaderManager::s_staticShaders;

GLShader* GLShaderManager::addShader(
    const char* uniqueName,
    const fs::path& vertexShaderPath,
    const fs::path& fragmentShaderPath)
{

    bool success = true;
    std::string vertexShaderCode;
    std::ifstream vertexShaderStream(vertexShaderPath, std::ios::in);

    if (vertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << vertexShaderStream.rdbuf();
        vertexShaderCode = sstr.str();
        vertexShaderStream.close();
    } else {
        success = false;
        LOGE("Can't open file: " << vertexShaderPath);
    }

    std::string fragmentShaderCode;
    std::ifstream fragmentShaderStream(fragmentShaderPath, std::ios::in);
    if (fragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << fragmentShaderStream.rdbuf();
        fragmentShaderCode = sstr.str();
        fragmentShaderStream.close();
    } else {
        success = false;
        LOGE("Can't open file: " << fragmentShaderPath);
    }

    if (success) {
        return addShader(uniqueName, vertexShaderCode, fragmentShaderCode);
    }
    return nullptr;
}

GLShader* GLShaderManager::addShader(
    const char* uniqueName,
    const char* vertexShaderCode,
    const char* fragmentShaderCode)
{

    auto foundShader = getByName(uniqueName);
    if (foundShader) {
        LOGE("Shader with this name already exists, get from cache");
        return foundShader;
    } else {
        uint32_t shaderProgram;
        UniformList uniforms;
        if (createShaderProgram(vertexShaderCode, fragmentShaderCode, shaderProgram, uniforms)) {
            s_staticShaders[uniqueName].initialize(shaderProgram, std::move(uniforms));
            LOG("Shader: \"" << uniqueName << "\" created successfully.");
            return &s_staticShaders[uniqueName];

        } else {
            LOGE("Failed to compile shader");
            return nullptr;
        }
    }

    return nullptr;
}

GLShader* GLShaderManager::getDefaultShader2d()
{
    static constexpr const char* defaultShaderName = "DefaultShader2d";
    auto shader = getByName(defaultShaderName);
    if (shader)
        return shader;
    else
        return addShader(defaultShaderName,
            GLShaderSources::getDefault2d_VS(),
            GLShaderSources::getDefault2d_FS());
}

GLShader* GLShaderManager::getByName(HashString str)
{
    auto it = s_staticShaders.find(str);
    if (it != s_staticShaders.end()) {
        return &it->second;
    }
    return nullptr;
}
