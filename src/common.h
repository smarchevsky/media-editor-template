#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <variant>

static std::size_t constexpr constexpr_hash(char const* input)
{
    return *input ? static_cast<std::size_t>(*input) + 33 * constexpr_hash(input + 1) : 5381;
}

inline void hash_combine(std::size_t& seed) { }
template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}

/// https://gist.github.com/nnaumenko/1db96f7e187979a057ee7ad757dee4f2
/// @return Index of type T in variant V; or variant size if V does not include an alternative for T.
template <typename V, typename T, std::size_t I = 0>
constexpr std::size_t variant_index()
{
    if constexpr (I >= std::variant_size_v<V>) {
        return (std::variant_size_v<V>);
    } else {
        if constexpr (std::is_same_v<std::variant_alternative_t<I, V>, T>) {
            return (I);
        } else {
            return (variant_index<V, T, I + 1>());
        }
    }
}

static std::string textFromFile(const std::filesystem::path& path)
{
    std::string sourceCode;
    std::ifstream codeStream(path, std::ios::in);
    if (codeStream.is_open()) {
        std::stringstream sstr;
        sstr << codeStream.rdbuf();
        sourceCode = sstr.str();
        codeStream.close();
    } else {
        printf("Can't open file: %s", path.c_str());
    }
    return sourceCode;
}

#define GET_INDEX(type) variant_index<UniformVariant, type>()

#define UNCONST(variable) const_cast<std::remove_const_t<decltype(variable)>&>(variable)

#endif // COMMON_H
