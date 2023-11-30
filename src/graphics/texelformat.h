#ifndef TEXELFORMAT_H
#define TEXELFORMAT_H

// #include <cstdint>
#include <glm/glm.hpp>
#include <variant>

// clang-format off
enum class TexelFormat : uint8_t { Undefined,
    R_8, RGB_8, RGBA_8, R_32F, RGB_32F, RGBA_32F
};
// clang-format on

struct TexelFormatInfo {
    TexelFormatInfo(TexelFormat format);
    const char* name;
    int glInternalFormat; // e.g. GL_RGB8
    int glExternalFormat; // e.g. GL_RGB
    int glExternalType; //   e.g. GL_UNSIGNED_BYTE
    uint8_t numChannels;
    uint8_t sizeInBytes;
};

// struct Texel {
//     // clang-format off
//     explicit Texel(uint8_t     r8)      :data(r8)      ,format(TexelFormat::R_8)      {}
//     explicit Texel(glm::u8vec3 rgb8)    :data(rgb8)    ,format(TexelFormat::RGB_8)    {}
//     explicit Texel(glm::u8vec4 rgba8)   :data(rgba8)   ,format(TexelFormat::RGBA_8)   {}
//     explicit Texel(float       r32f)    :data(r32f)    ,format(TexelFormat::R_32F)    {}
//     explicit Texel(glm::vec3   rgb32f)  :data(rgb32f)  ,format(TexelFormat::RGB_32F)  {}
//     explicit Texel(glm::vec4   rgba32f) :data(rgba32f) ,format(TexelFormat::RGBA_32F) {}
//     // clang-format on
//     std::variant<uint8_t, glm::u8vec3, glm::u8vec4, float, glm::vec3, glm::vec4> data;
//     TexelFormat format;
// };

#endif // TEXELFORMAT_H
