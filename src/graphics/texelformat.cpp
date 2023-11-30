#include "texelformat.h"
#include <cassert>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

TexelFormatInfo::TexelFormatInfo(TexelFormat format)
{
    switch (format) {
    case TexelFormat::R_8: {
        name = "R_8";
        glInternalFormat = GL_RED;
        glExternalFormat = GL_RED;
        glExternalType = GL_UNSIGNED_BYTE;
        numChannels = 1;
        sizeInBytes = 1;
    } break;

    case TexelFormat::RGB_8: {
        name = "RGB_8";
        glInternalFormat = GL_RGB8;
        glExternalFormat = GL_RGB;
        glExternalType = GL_UNSIGNED_BYTE;
        numChannels = 3;
        sizeInBytes = 3;
    } break;

    case TexelFormat::RGBA_8: {
        name = "RGBA_8";
        glInternalFormat = GL_RGBA8;
        glExternalFormat = GL_RGBA;
        glExternalType = GL_UNSIGNED_BYTE;
        numChannels = 4;
        sizeInBytes = 4;
    } break;

    case TexelFormat::R_32F: {
        name = "R_32F";
        glInternalFormat = GL_R32F;
        glExternalFormat = GL_RED;
        glExternalType = GL_FLOAT;
        numChannels = 1;
        sizeInBytes = 4;
    } break;

    case TexelFormat::RGB_32F: {
        name = "RGB_32F";
        glInternalFormat = GL_RGB32F;
        glExternalFormat = GL_RGB;
        glExternalType = GL_FLOAT;
        numChannels = 3;
        sizeInBytes = 12;
    } break;

    case TexelFormat::RGBA_32F: {
        name = "RGBA_32F";
        glInternalFormat = GL_RGBA32F;
        glExternalFormat = GL_RGBA;
        glExternalType = GL_FLOAT;
        numChannels = 4;
        sizeInBytes = 16;
    } break;

    default:
        // LOGE("Texture format enum: " << (int)format << " not supported");
        assert(false && "Texture format not supported");
    }
}
