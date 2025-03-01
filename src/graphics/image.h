#ifndef IMAGE_H
#define IMAGE_H
#include "common.h"
#include "texelformat.h"

#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>

class Image : NoCopy<Image> {
protected:
    struct STBIDeleter {
        void operator()(uint8_t* data) const;
    };

protected:
    std::unique_ptr<uint8_t, STBIDeleter> m_data;
    glm::ivec2 m_size = glm::ivec2(0);
    TexelFormat m_format {};
    std::string m_name;

public:
    Image() = default;
    Image(const std::filesystem::path& path) { load(path); }
    Image& operator=(Image&& rhs)
    {
        if (this != &rhs) {
            m_data = std::move(rhs.m_data);
            m_size = rhs.m_size;
            m_format = rhs.m_format;
            m_name = std::move(rhs.m_name);
        }
        return *this;
    }
    Image(Image&& rhs) { *this = std::move(rhs); }

    Image(glm::ivec2 size, TexelFormat format); // uninitialized
    explicit Image(glm::ivec2 size, int32_t packedRGBA) { fill(size, packedRGBA); }
    ~Image();

    void load(const std::filesystem::path& path);
    void fill(glm::ivec2 size, int32_t packedColor);
    void clear();
    bool isValid() const;

    bool writeToFile(const std::filesystem::path& path, bool flipVertically = false) const;

    uint8_t* getDataMutable() { return m_data.get(); }
    const uint8_t* getData() const { return m_data.get(); }
    size_t getDataSize() const;

    glm::ivec2 getSize() const { return m_size; }

    TexelFormat getFormat() const { return m_format; }

    friend class GLTexture2D;
};

#endif // IMAGE_H
