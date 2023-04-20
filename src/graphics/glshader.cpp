#include "glshader.h"
#include "glshadersources.h"
#include "helper_general.h"

#define ERRORCHEKING
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace {
UniformLocationMap getUniformList(GLuint program)
{
    UniformLocationMap result;
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
    }
    */
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);

    for (int i = 0; i < count; i++) {
        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
        result.insert({ name, i });
        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
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
    uint32_t& outShaderProgram, UniformLocationMap& outUniforms)
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

}

uint32_t GLShader::s_currentBindedShaderHandle = 0;
std::unordered_map<HashString, GLShader> GLShaderCompiler::s_staticShaders;

void GLShader::setUniform(int location, float var) { glUniform1f(location, var); }
void GLShader::setUniform(int location, const glm::vec2& var) { glUniform2fv(location, 1, &var[0]); }
void GLShader::setUniform(int location, const glm::vec3& var) { glUniform3fv(location, 1, &var[0]); }
void GLShader::setUniform(int location, const glm::vec4& var) { glUniform4fv(location, 1, &var[0]); }
void GLShader::setUniform(int location, const glm::mat4& var) { glUniformMatrix4fv(location, 1, GL_FALSE, &var[0][0]); }
void GLShader::setUniform(int location, const GLTexture& texture) { glUniform1i(location, texture.getHandle()); }

int GLShader::getUniformLocation(HashString name)
{
    // return glGetUniformLocation(m_shaderProgram, name);
    auto it = m_uniforms.find(name);
    if (it != m_uniforms.end()) {
        return it->second;
    }
    return -1;
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

void GLShader::bind()
{
//#define ALWAYS_BIND_SHADER
#ifdef ALWAYS_BIND_SHADER
    glUseProgram(m_shaderProgram);
#else
    if (s_currentBindedShaderHandle != m_shaderProgram) {
        s_currentBindedShaderHandle = m_shaderProgram;
        glUseProgram(m_shaderProgram);
    }
#endif
}

void GLShaderInstance::setShader(GLShader* shader)
{
    m_shader = shader;
}

GLShader* GLShaderCompiler::addShader(
    const char* vertexShaderCode, const char* fragmentShaderCode, const char* uniqueName)
{

    const auto it = s_staticShaders.find(uniqueName);
    if (it == s_staticShaders.end()) {

        uint32_t shaderProgram;
        UniformLocationMap uniforms;
        if (createShaderProgram(vertexShaderCode, fragmentShaderCode, shaderProgram, uniforms)) {
            s_staticShaders[uniqueName].initialize(shaderProgram, uniforms);
            LOG("Shader: \"" << uniqueName << "\" created successfully.");
            return &s_staticShaders[uniqueName];

        } else {
            LOGE("Failed to compile shader");
            return nullptr;
        }
    } else {
        return &it->second;
        LOGE("Shader with this name already exists, get from cache");
    }
    return nullptr;
}

GLShader* GLShaderCompiler::addShader(
    const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath, const char* uniqueName)
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
        std::cerr << "Can't open file: " << vertexShaderPath << std::endl;
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
        std::cerr << "Can't open file: " << fragmentShaderPath << std::endl;
    }

    if (success) {
        return addShader(vertexShaderCode, fragmentShaderCode, uniqueName);
    }
    return nullptr;
}

GLShader* GLShaderCompiler::getDefaultShader2d()
{
    return addShader(GLShaderSources::getDefault2d_VS(), GLShaderSources::getDefault2d_FS(), "DefaultShader2d");
}

GLShader* GLShaderCompiler::getByName(HashString str)
{
    auto it = s_staticShaders.find(str);
    if (it != s_staticShaders.end()) {
        return &it->second;
    }
    return nullptr;
}
