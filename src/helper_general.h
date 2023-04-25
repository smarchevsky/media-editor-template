#ifndef HELPER_GENERAL_H
#define HELPER_GENERAL_H

template <class T>
class NoCopy {
public:
    NoCopy(const NoCopy&) = delete;
    T& operator=(const T&) = delete;

protected:
    NoCopy() = default;
    ~NoCopy() = default; /// Protected non-virtual destructor
};
inline const char* getPathFromSourceDirectory(const char* path){
    return path + SOURCE_PATH_SIZE;
}

#define CAST(Type, ptr) std::dynamic_pointer_cast<Type>(ptr)
// clang-format off
#define LOG(x)  std::cout << getPathFromSourceDirectory(__FILE__) << " line: " << __LINE__ << ". " << x << std::endl
#define LOGE(x) std::cerr << getPathFromSourceDirectory(__FILE__) << " line: " << __LINE__ << ". " << x << std::endl
// fprintf(stderr, "%2.2x", var[i]);
//#define LOG(text, ...) UE_LOG(LogTemp, Warning,
// TEXT("%s: " text), LOG_FUNCTION_COMMENT, ##__VA_ARGS__)

#endif
