#ifndef GLSHADER_H
#define GLSHADER_H

#include "gl_texture.h"
#include "hashstring.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstring>
#include <filesystem>

#include <string>
#include <unordered_map>

#include <variant>
#include <vector>

static const std::filesystem::path shaderDir(SHADER_DIR);

struct Texture2Ddata {
    Texture2Ddata() = default;
    Texture2Ddata(const std::shared_ptr<GLTexture2D> texture, int index = -1)
        : m_texture(texture)
        , m_index(index)
    {
    }
    std::shared_ptr<GLTexture2D> m_texture;
    int m_index = -1;
};

#ifdef UNIFORM_BUFFER_FEATURE
// I don't want to create separate branch and deal with merge conflicts

class UniformStruct {
public:
    // clang-format off
    enum class DataPackingFormat : uint8_t { Invalid, CPU, std140, std430 } m_packingFormat {};
    struct MetaData { uint8_t type; uint16_t offset; };
    struct SizeAlign { int size; int align; };

protected:
    std::vector<uint8_t> m_data; // all data in bytes
    std::vector<MetaData> m_metaData; // size: num arguments
    // clang-format on

public:
    template <typename... Args>
    explicit UniformStruct(Args... args); // implementation after UniformVariant
};
#endif

typedef std::variant<
    char, // is invalid, dont set char :)

    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4,

    std::vector<float>,
    std::vector<glm::vec2>,
    std::vector<glm::vec3>,
    std::vector<glm::vec4>,
    std::vector<glm::mat4>,

    int,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,

    Texture2Ddata>

    UniformVariant;

#ifdef UNIFORM_BUFFER_FEATURE
template <typename... Args>
UniformStruct::UniformStruct(Args... args)
{
    constexpr size_t numArg = sizeof...(args);
    m_metaData.resize(numArg);

    auto getSizeAndAlignForStd140 = [](size_t uniformVarIndex) -> SizeAlign {
        switch (uniformVarIndex) {
        case GET_UNIFORM_VARIANT_INDEX(float):
        case GET_UNIFORM_VARIANT_INDEX(int): {
            return { 4, 4 };
        }
        case GET_UNIFORM_VARIANT_INDEX(glm::vec2):
        case GET_UNIFORM_VARIANT_INDEX(glm::ivec2): {
            return { 8, 8 };
        }
        case GET_UNIFORM_VARIANT_INDEX(glm::vec3):
        case GET_UNIFORM_VARIANT_INDEX(glm::ivec3):
        case GET_UNIFORM_VARIANT_INDEX(glm::vec4):
        case GET_UNIFORM_VARIANT_INDEX(glm::ivec4): {
            return { 16, 16 };
        }
        case GET_UNIFORM_VARIANT_INDEX(glm::mat4): {
            return { 64, 16 };
        }
        default:
            assert(false && "Not supported format for uniform variable");
            return { 0, 0 };
        }
    };

    /*auto updateOffset = [](size_t& offset, size_t size, size_t align) {
        size_t padding = (align - (offset % align)) % align;
        offset += padding + size;
    };*/

    size_t currentOffset = 0, currentArgIndex = 0;
    auto computeDataSizeAndMetadata = [&](size_t typeIndex, auto&& arg) {
        assert(typeIndex < std::numeric_limits<decltype(MetaData::type)>().max());
        assert(currentOffset < std::numeric_limits<decltype(MetaData::offset)>().max());
        SizeAlign dataSizeAlign = getSizeAndAlignForStd140(typeIndex);

        size_t padding = (dataSizeAlign.align - (currentOffset % dataSizeAlign.align)) % dataSizeAlign.align;
        currentOffset += padding;

        auto& md = m_metaData[currentArgIndex];
        md.type = static_cast<decltype(MetaData::type)>(typeIndex);
        md.offset = currentOffset;
        LOGE(currentArgIndex << " type: " << (int)md.type << " offset: " << md.offset);

        currentOffset += dataSizeAlign.size;
        currentArgIndex++;
    };

    (computeDataSizeAndMetadata(GET_UNIFORM_VARIANT_INDEX(decltype(args)), args), ...);

    m_data.resize(currentOffset);
    std::memset(m_data.data(), 0, m_data.size()); // clear old data

    currentOffset = currentArgIndex = 0;
    auto pushData = [&](auto&& arg) {
        std::memcpy(m_data.data() + m_metaData[currentArgIndex].offset, &arg, sizeof(arg));

        currentArgIndex++;
    };

    (pushData(args), ...);
    m_packingFormat = DataPackingFormat::CPU;

    std::cout << std::endl;
    for (int i = 0; i < m_data.size() / 4; ++i) {
        float curr = ((float*)m_data.data())[i];
        LOG(curr);
    }

    exit(0);
}
#endif

typedef std::unordered_map<HashString, UniformVariant> UniformContainer;
//////////////////////// SHADER //////////////////////////

class GLShader : NoCopy<GLShader> {

public:
    static GLShader FromFile(
        const std::filesystem::path& vertRelativePath,
        const std::filesystem::path& fragRelativePath);

    int getHandle() const { return m_shaderProgram; }

    // binds shader and clears uniforms
    void bindAndResetUniforms();

    // set uniforms by objects, if variable was previously marked as Camera type - assert
    void setUniforms(const UniformContainer& newUniforms);

    // set camera uniforms right after bind reset uniforms
    void setCameraUniforms(const UniformContainer& cameraUniforms);

    GLShader() = default;
    GLShader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
    ~GLShader();

    // clang-format off
    enum class UniformType : uint8_t { Default, Camera };
    enum class UniformStatus : uint8_t { DontTouch, MustUpdate, MustResetToDefault };

    class Variable {
        UniformVariant m_defaultData;
        mutable UniformVariant m_currentData;
        int m_location = -1;
        std::string m_name;
        mutable UniformType m_type = UniformType::Default;
        mutable UniformStatus m_status = UniformStatus::MustUpdate; // variables are dirty by default, to update in shader state machine

    public:
        Variable() = default;
        Variable(int location, const std::string& m_name, UniformVariant data)
            : m_defaultData(data), m_currentData(data),  m_location(location), m_name(m_name)  {}

        friend class GLShader;
        friend class ShaderCodeParser;
    };
    // clang-format on
private:
    // UniformType getCurrentUniformType(int location) const { return m_uniforms[location]; }

    void setUniformInternal(int location, const UniformVariant& var);

public:
    GLShader& operator=(GLShader&& rhs)
    {
        const_cast<uint32_t&>(m_shaderProgram) = rhs.m_shaderProgram;
        const_cast<uint32_t&>(rhs.m_shaderProgram) = 0;

        const_cast<std::unordered_map<HashString, int>&>(m_locations) = std::move(rhs.m_locations);
        const_cast<std::unordered_map<int, Variable>&>(m_uniforms) = std::move(rhs.m_uniforms);
        return *this;
    }

private: // DATA
    const uint32_t m_shaderProgram {};

    const std::unordered_map<HashString, int> m_locations;
    const std::unordered_map<int, Variable> m_uniforms;

private:
    static uint32_t s_currentBindedShaderHandle;
    friend class GLShaderManager;
};

#endif // SHADER_H
