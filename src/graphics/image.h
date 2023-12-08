#ifndef IMAGE_H
#define IMAGE_H
#include "helper_general.h"
#include "texelformat.h"

#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>

class Image : NoCopy<Image> {
    uint8_t* m_data {};
    glm::ivec2 m_size = glm::ivec2(0);
    TexelFormat m_format {};
    std::string m_name;

public:
    Image(Image&& rhs)
    {
        m_data = rhs.m_data, rhs.m_data = nullptr;

        m_size = rhs.m_size;
        m_format = rhs.m_format;
        m_name = rhs.m_name;
    }

    Image() { }
    Image(const std::filesystem::path& path) { load(path); }
    explicit Image(glm::ivec2 size, int32_t packedRGBA) { fill(size, packedRGBA); }
    ~Image();

    void load(const std::filesystem::path& path);
    void fill(glm::ivec2 size, int32_t packedColor);
    void clear();
    bool isValid() const;

    unsigned char* getDataMutable() { return m_data; }
    const unsigned char* getData() const { return m_data; }

    size_t getDataSize() const;
    glm::ivec2 getSize() const { return m_size; }

    TexelFormat getFormat() const { return m_format; }

    friend class GLTexture2D;
};

#endif // IMAGE_H
