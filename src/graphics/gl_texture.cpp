#include "gl_texture.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <iostream>
// #include <cassert>

uint32_t GLTexture2D::s_currentBindedTexture = 0;

namespace {
struct TexelFormatInfo {
    int internalFormat; // GL_RGB8
    int externalFormat; // GL_RGB
    int externalType; //   GL_UNSIGNED_BYTE
    std::string name;
};

TexelFormatInfo getGLTexelFormatInfo(GLTexture2D::Format format)
{
    TexelFormatInfo texelFormat;
    switch (format) {
    case GLTexture2D::Format::RGB_8: {
        texelFormat.internalFormat = GL_RGB8;
        texelFormat.externalFormat = GL_RGB;
        texelFormat.externalType = GL_UNSIGNED_BYTE;
        texelFormat.name = "RGB_8";
    } break;

    case GLTexture2D::Format::RGBA_8: {
        texelFormat.internalFormat = GL_RGBA8;
        texelFormat.externalFormat = GL_RGBA;
        texelFormat.externalType = GL_UNSIGNED_BYTE;
        texelFormat.name = "RGBA_8";
    } break;

    case GLTexture2D::Format::RGB_32F: {
        texelFormat.internalFormat = GL_RGB32F;
        texelFormat.externalFormat = GL_RGB;
        texelFormat.externalType = GL_FLOAT;
        texelFormat.name = "RGB_32F";
    } break;

    case GLTexture2D::Format::RGBA_32F: {
        texelFormat.internalFormat = GL_RGBA32F;
        texelFormat.externalFormat = GL_RGBA;
        texelFormat.externalType = GL_FLOAT;
        texelFormat.name = "RGBA_32F";
    } break;

    default:
        LOGE("Texture format enum: " << (int)format << " not supported");
        assert(false && "Stop it!");
    }
    return texelFormat;
}
}

/////////////////// TEXTURE 2D ////////////////////

bool GLTexture2D::setWrapping(Wrapping wrapping)
{
    if (!m_textureHandle) {
        LOGE("No texture handle");
        return false;
    }

    int wrappingGLformat {};
    switch (wrapping) {
    case Wrapping::Repeat: {
        wrappingGLformat = GL_REPEAT;
    } break;
    case Wrapping::MirrorRepeat: {
        wrappingGLformat = GL_MIRRORED_REPEAT;
    } break;
    case Wrapping::ClampEdge: {
        wrappingGLformat = GL_CLAMP_TO_EDGE;
    } break;
    case Wrapping::ClampBorder: {
        wrappingGLformat = GL_CLAMP_TO_BORDER;
    } break;
    default:
        LOGE("Unsuppoerted wrapping type");
        return false;
    }

    m_wrapping = wrapping;
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingGLformat);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingGLformat);
    return true;
}

bool GLTexture2D::setFiltering(Filtering filtering)
{
    if (!m_textureHandle) {
        LOGE("No texture handle");
        return false;
    }

    int minFilter, magFilter;
    switch (filtering) {
    case Filtering::Nearset: {
        minFilter = GL_NEAREST;
        magFilter = GL_NEAREST;
    } break;

    case Filtering::NearestMipmap: {
        minFilter = GL_NEAREST_MIPMAP_LINEAR;
        magFilter = GL_NEAREST;
    } break;

    case Filtering::Linear: {
        minFilter = GL_LINEAR;
        magFilter = GL_LINEAR;
    } break;

    case Filtering::LinearMipmap: {
        minFilter = GL_LINEAR_MIPMAP_LINEAR;
        magFilter = GL_LINEAR;

    } break;
    default:
        LOGE("Unsupportering texture filtering type");
        return false;
    }

    m_filtering = filtering;

    generateMipMap();
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    return true;
}

bool GLTexture2D::createFromRawData(glm::ivec2 size, GLTexture2D::Format format, void* data)
{
    if (size.x <= 0 || size.y <= 0)
        assert(false && "Negative texture size is not good");

    clear();
    m_size = size;
    m_format = format;
    auto texelInfo = getGLTexelFormatInfo(m_format);

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0,
        texelInfo.internalFormat,
        size.x, size.y, 0,
        texelInfo.externalFormat,
        texelInfo.externalType,
        data);

    setFiltering(Filtering::Nearset);
    if (data) {
        LOG("Texture successfully created: " << texelInfo.name);
    } else {
        LOG("Empty texture successfully created: " << texelInfo.name);
    }

    return true;
}

bool GLTexture2D::fromImage(const Image& img)
{
    if (!img.isValid()) {
        LOGE("trying to load invalid texture");
        return false;
    }

    Format format;
    switch (img.m_nrChannels) {
    case 3: {
        format = Format::RGB_8;
    } break;

    case 4: {
        format = Format::RGBA_8;
    } break;

    default:
        assert(false && "Stop it");
        break;
    }

    createFromRawData(img.m_size, format, img.m_data);
    setFiltering(Filtering::LinearMipmap);
    setWrapping(Wrapping::Repeat);
    return true;
}

// void GLTexture2D::toImage(Image& img) const
// {
//     // float* data = new float[m_size.x * m_size.y * 3];
//     // glGetTextureImage(m_textureHandle, 0, GL_RGB, GL_FLOAT, m_size.x * m_size.y * 3 * 4, data);
// }

static bool isMipMap(GLTexture2D::Filtering filtering)
{
    return filtering == GLTexture2D::Filtering::NearestMipmap
        || filtering == GLTexture2D::Filtering::LinearMipmap;
}

void GLTexture2D::generateMipMap()
{
    // glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST /*GL_FASTEST*/);
    if (m_textureHandle && isMipMap(m_filtering)) {
        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void GLTexture2D::clear()
{
    if (m_textureHandle) {
        glDeleteTextures(1, &m_textureHandle);
        m_textureHandle = 0;
        m_size = glm::ivec2(0);
        m_format = Format::Undefined;
        LOG("Texture destroyed");
    }
}

GLTexture2D::~GLTexture2D() { clear(); }

/////////////////// RENDER (DEPTH) BUFFER /////////////////

GLDepthBuffer2D::GLDepthBuffer2D(glm::vec2 size)
{
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
}

GLDepthBuffer2D::~GLDepthBuffer2D()
{
    if (m_rbo) {
        glDeleteTextures(1, &m_rbo);
        m_rbo = 0;
        LOG("Render (depth) buffer destroyed");
    }
}
