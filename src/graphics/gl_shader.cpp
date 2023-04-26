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

static UniformDependency getDependencyTypeFromName(const std::string& name)
{
    UniformDependency dependency = UniformDependency::Object;
    if (name.find("view_") == 0)
        dependency = UniformDependency::View;
    else if (name.find("free_") == 0)
        dependency = UniformDependency::Free;
    return dependency;
}

UniformVariant createDefaultUniformData(int GLtype, int size, int textureIndex)
{
    // constexpr int floatSize = sizeof(float);
    // constexpr int intSize = sizeof(int);

    switch (GLtype) {
    case GL_FLOAT: {
        // assert(size == floatSize);
        return 0.f;
    };

    case GL_FLOAT_VEC2: {
        // assert(size == floatSize * 2);
        return glm::vec2(0);
    }

    case GL_FLOAT_VEC3: {
        // assert(size == floatSize * 3);
        return glm::vec3(0);
    };

    case GL_FLOAT_VEC4: {
        // assert(size == floatSize * 4);
        return glm::vec4(0);
    };

    case GL_SAMPLER_2D: {
        // assert(size == intSize);
        return Texture2Ddata(std::shared_ptr<GLTexture>(), textureIndex);
    }

    case GL_FLOAT_MAT4: {
        // LOGE("size of mat4 is: " << size);
        return glm::mat4(1);
    };
    }

    assert(false && "Unsupported uniform format");
    return {};
}

GLShader::UniformVariables getUniformList(GLShader* shader)
{

    GLint count;
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 512; // maximum name length
    GLchar name[bufSize] {}; // variable name in GLSL
    GLsizei length; // name length

    /* glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
    printf("Active Attributes: %d\n", count);
    for (int i = 0; i < count; i++)
    {
        glGetActiveAttrib(program, (GLuint)i, bufSize, &length, &size, &type, name);
        printf("Attribute #%d Type: %u Name: %s\n", i, type, name);
    }*/

    const auto program = shader->getHandle();
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);

    GLShader::UniformVariables result;

    int texture2DIndex = 0;
    for (int i = 0; i < count; i++) {
        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
        std::string nameStr(name);

        GLShader::Variable var(i, nameStr,
            getDependencyTypeFromName(nameStr),
            createDefaultUniformData(type, size, texture2DIndex));

        result.insert({ name, std::move(var) });

        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);

        if (type == GL_SAMPLER_2D)
            texture2DIndex++;
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

} // namespace

GLShader::GLShader(const char* vertexShaderCode, const char* fragmentShaderCode)
{
    GLint success = GL_FALSE;
    if (vertexShaderCode && fragmentShaderCode) {
        m_shaderProgram = glCreateProgram();
        GLuint vs = createShader(vertexShaderCode, GL_VERTEX_SHADER);
        GLuint fs = createShader(fragmentShaderCode, GL_FRAGMENT_SHADER);
        glAttachShader(m_shaderProgram, vs);
        glAttachShader(m_shaderProgram, fs);
        glLinkProgram(m_shaderProgram);

        char infoLog[512];
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

        if (success == GL_TRUE) {

            const_cast<UniformVariables&>(m_defaultVariables) = getUniformList(this);

        } else {

            glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
            std::cerr << "Linking failed: " << infoLog << std::endl;
            m_shaderProgram = 0;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
}

int GLShader::getUniformLocation(const HashString &name) const
{
    return glGetUniformLocation(m_shaderProgram, name.getString().c_str());
}

GLShader::~GLShader()
{
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        LOG("Shader destroyed");
    }
}

void GLShader::setUniform(const HashString &name, const UniformVariant& var)
{
    bind();
    auto it = m_defaultVariables.find(name);
    if (it != m_defaultVariables.end()) {
        int location = it->second.getLocation();
        setUniform(location, var);
    } else {
        LOGE("Variable: " << name.getString() << " did not find in shader.");
    }
}



#define GET_INDEX(type) variant_index<UniformVariant, type>()
void GLShader::setUniform(int location, const UniformVariant& uniformVariable)
{
    if (uniformVariable.index())
        switch (uniformVariable.index()) {

        case GET_INDEX(Texture2Ddata): {
            const auto& var = std::get<Texture2Ddata>(uniformVariable);

            const auto& sharedTexture = std::get<0>(var);
            int textureIndex = sharedTexture ? std::get<1>(var) : 0;
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

        default: {
            LOGE("Unsupported uniform variable type");
        }
        }
}

void GLShader::Variable::setData(const std::shared_ptr<GLTexture>& newTexture)
{
    if (m_data.index() == GET_INDEX(Texture2Ddata)) {
        Texture2Ddata& existedTexture = std::get<Texture2Ddata>(m_data);
        std::get<0>(existedTexture) = newTexture;
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

//////////////////////// SHADER MANAGER //////////////////////////

GLShaderPtr GLShaderManager::addShader(
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

GLShaderPtr GLShaderManager::addShader(
    const char* uniqueName,
    const char* vertexShaderCode,
    const char* fragmentShaderCode)
{

    auto foundShader = getByName(uniqueName);
    if (foundShader) {
        LOGE("Shader with this name already exists, get from cache");
        return foundShader;
    } else {
        GLShaderPtr shader = std::make_shared<GLShader>(vertexShaderCode, fragmentShaderCode);
        if (shader->valid()) {
            s_staticShaders[uniqueName] = shader;
            LOG("Shader: \"" << uniqueName << "\" created successfully.");
            return shader;
        } else {
            LOGE("Failed to compile shader");
            return nullptr;
        }
    }

    return nullptr;
}

GLShaderPtr GLShaderManager::getDefaultShader2d()
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

GLShaderPtr GLShaderManager::getByName(const HashString &name)
{
    auto it = s_staticShaders.find(name);
    if (it != s_staticShaders.end()) {
        return it->second;
    }
    return nullptr;
}

void GLShader::Variable::setData(const UniformVariant& data)
{
    if (isValid()) {
        if (m_data.index() == data.index()) {
            m_data = data;
        } else {
            LOGE("You are trying to set invalid data type for variable: " << m_name);
        }
    } else {
        LOGE("Variable: " << m_name << " does not exist in this shader");
    }
}
