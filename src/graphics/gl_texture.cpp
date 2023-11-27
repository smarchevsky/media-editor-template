#include "gl_texture.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <iostream>
// #include <cassert>

// uint32_t GLTexture2D::s_currentBindedTextureHandle = 0;

namespace {
struct TexelFormatInfo {
    int internalFormat; // GL_RGB8
    int externalFormat; // GL_RGB
    int externalType; //   GL_UNSIGNED_BYTE
    const char* name;
};

TexelFormatInfo getGLTexelFormatInfo(GLTexture2D::Format format)
{
    TexelFormatInfo texelFormat;
    switch (format) {

    case GLTexture2D::Format::R_8: {
        texelFormat.internalFormat = GL_RED;
        texelFormat.externalFormat = GL_RED;
        texelFormat.externalType = GL_UNSIGNED_BYTE;
        texelFormat.name = "R_8";
    } break;

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

    case GLTexture2D::Format::R_32F: {
        texelFormat.internalFormat = GL_R32F;
        texelFormat.externalFormat = GL_RED;
        texelFormat.externalType = GL_FLOAT;
        texelFormat.name = "R_32F";
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
        assert(false);
    }

    return texelFormat;
}
}

/////////////////// TEXTURE 2D ////////////////////

void GLTexture2D::setWrapping(Wrapping wrapping)
{
    if (!m_textureHandle)
        assert(false && "No texture handle");

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
        assert(false && "Unsuppoerted wrapping type");
    }

    m_wrapping = wrapping;

    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingGLformat);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingGLformat);
}

void GLTexture2D::setFiltering(Filtering filtering)
{
    if (!m_textureHandle) {
        assert(false && "No texture handle");
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
        assert(false && "Unsupportering texture filtering type");
    }

    m_filtering = filtering;
    m_mipmapDirty = true;

    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void GLTexture2D::createFromRawData(glm::ivec2 size, GLTexture2D::Format format, void* data)
{
    m_size = size;
    m_format = format;
    auto texelInfo = getGLTexelFormatInfo(m_format);

    if (!m_textureHandle)
        glGenTextures(1, &m_textureHandle);

    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    glTexImage2D(GL_TEXTURE_2D, 0,
        texelInfo.internalFormat,
        size.x, size.y, 0,
        texelInfo.externalFormat,
        texelInfo.externalType,
        data);

    setFiltering(Filtering::Nearset);

    LOG((data ? "Texture created: " : "Empty texture created: ") << texelInfo.name);
}

void GLTexture2D::updateData(void* data)
{
    if (!m_textureHandle)
        return;

    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    auto texelInfo = getGLTexelFormatInfo(m_format);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
        0, 0, m_size.x, m_size.y,
        texelInfo.externalFormat,
        texelInfo.externalType,
        data);
}

bool GLTexture2D::fromImage(const Image& img)
{
    if (!img.isValid()) {
        LOGE("trying to load invalid texture");
        return false;
    }

    // clang-format off
    Format format = Format::Undefined;
    switch (img.m_nrChannels) {
        case 1: { format = Format::R_8; } break;
        case 3: { format = Format::RGB_8; } break;
        case 4: { format = Format::RGBA_8; } break;
        default: assert(false && "This num of channels (probably 2) does not support");
    }
    // clang-format on

    createFromRawData(img.m_size, format, img.m_data);
    setFiltering(Filtering::LinearMipmap);
    setWrapping(Wrapping::Repeat);

    return true;
}

static bool isMipMap(GLTexture2D::Filtering filtering)
{
    return filtering == GLTexture2D::Filtering::NearestMipmap
        || filtering == GLTexture2D::Filtering::LinearMipmap;
}

void GLTexture2D::generateMipMapsIfDirty()
{
    // glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST /*GL_FASTEST*/);
    if (m_mipmapDirty && isMipMap(m_filtering) && m_textureHandle) {
        glBindTexture(GL_TEXTURE_2D, m_textureHandle); // force bind
        glGenerateMipmap(GL_TEXTURE_2D);
        m_mipmapDirty = false;
    }
}

GLTexture2D::~GLTexture2D()
{
    if (m_textureHandle) {
        glDeleteTextures(1, &m_textureHandle);
        m_textureHandle = 0;
        LOG("Texture destroyed");
    }
}

// void GLTexture2D::bind()
// {
//     if (s_currentBindedTextureHandle != m_textureHandle) {
//         glBindTexture(GL_TEXTURE_2D, m_textureHandle);
//         s_currentBindedTextureHandle = m_textureHandle;
//     }
// }

/////////////////// RENDER (DEPTH) BUFFER /////////////////

void GLDepthBuffer2D::create(glm::ivec2 size)
{
    if (!m_rbo)
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
