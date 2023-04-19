#include "glshader.h"
#include "helper_general.h"

#define ERRORCHEKING
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <fstream>
#include <iostream>
#include <sstream>

void GLShader::setUniform(const char* name, float var) { glUniform1f(getUniformLocation(name), var); }
void GLShader::setUniform(const char* name, const glm::vec2& var) { glUniform2fv(getUniformLocation(name), 1, &var[0]); }
void GLShader::setUniform(const char* name, const glm::vec3& var) { glUniform3fv(getUniformLocation(name), 1, &var[0]); }
void GLShader::setUniform(const char* name, const glm::vec4& var) { glUniform4fv(getUniformLocation(name), 1, &var[0]); }
void GLShader::setUniform(const char* name, const glm::mat4& var) { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &var[0][0]); }
void GLShader::setUniform(const char* name, const GLTexture& texture)
{
    glUniform1i(getUniformLocation(name), texture.getHandle());
}

int GLShader::getUniformLocation(const char* name)
{
    if (m_shaderProgramHandle) {
        int location = glGetUniformLocation(m_shaderProgramHandle, name);
        if (location == -1) {
            LOGE("No such variable: " << name << ", in shader: " << m_shaderVisualName);
        }

        return location;
    }
    // std::cerr << "Invalid shader: " << m_shaderVisualName << std::endl;
    return -1;
}

void GLShader::create(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath)
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

    // Read the Fragment Shader code from the file
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

    m_shaderVisualName = vertexShaderPath.filename().string() + " " + fragmentShaderPath.filename().string();

    if (success)
        create(vertexShaderCode, fragmentShaderCode);
}

void GLShader::create(const char* vertexShaderCode, const char* fragmentShaderCode, const char* shaderVisialName)
{
    if (vertexShaderCode && fragmentShaderCode) {
        int shaderProgramHandle = glCreateProgram();
        int vs = createShader(vertexShaderCode, GL_VERTEX_SHADER);
        int fs = createShader(fragmentShaderCode, GL_FRAGMENT_SHADER);
        glAttachShader(shaderProgramHandle, vs);
        glAttachShader(shaderProgramHandle, fs);
        glLinkProgram(shaderProgramHandle);

        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgramHandle, GL_LINK_STATUS, &success);
        if (success == GL_TRUE) {
            m_shaderProgramHandle = shaderProgramHandle;
        } else {
            glGetProgramInfoLog(shaderProgramHandle, 512, NULL, infoLog);
            std::cerr << "Linking failed: " << infoLog << std::endl;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    m_shaderVisualName = shaderVisialName;
}

GLShader::~GLShader()
{
    glDeleteProgram(m_shaderProgramHandle);
}

void GLShader::bind()
{
    glUseProgram(m_shaderProgramHandle);
}

int GLShader::createShader(const char* shaderSource, int shaderType)
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
