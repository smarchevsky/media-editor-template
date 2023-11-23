#ifndef IMAGE_H
#define IMAGE_H
#include "helper_general.h"
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Image : NoCopy<Image> {
public:
    Image();
    Image(const std::filesystem::path& path) { load(path); }
    explicit Image(glm::ivec2 size, int32_t packedRGBA) { fill(size, packedRGBA); }
    Image(Image&& rhs);
    ~Image();

    void load(const std::filesystem::path& path);
    void fill(glm::ivec2 size, int32_t packedColor);
    void clear();
    bool isValid() const;

    unsigned char* getDataMutable() { return m_data; }
    const unsigned char* getData() const { return m_data; }
    size_t getDataSize() const { return getSize().x * getSize().y * getNumChannels(); }
    int getNumChannels() const { return m_nrChannels; }
    glm::ivec2 getSize() const { return m_size; }

private:
    std::string m_name;
    glm::ivec2 m_size = glm::ivec2(0);
    int m_nrChannels {};
    unsigned char* m_data {};

    friend class GLTexture2D;
};

#endif // IMAGE_H
