#ifndef OPENFILEINFO_H
#define OPENFILEINFO_H
#include <filesystem>
#include <functional>
#include <optional>
class Application;
typedef std::function<bool(const std::filesystem::path&)> FileInteractionFunction;
struct OpenFileInfo {
    explicit OpenFileInfo(
        Application* const parentApp,
        const std::string& extensions,
        const std::filesystem::path& currentDir,
        FileInteractionFunction fileReader,
        FileInteractionFunction fileWriter);

    Application* const m_parentApp;
    const std::string m_supportedFileExtensions;

    std::filesystem::path m_currentDir;
    std::optional<std::filesystem::path> m_currentFileName;

    const FileInteractionFunction m_openFileFunction;
    const FileInteractionFunction m_saveFileFunction;

    void openFileDialog();
    // if m_openedFileName (and !forceDialogWindow) force save current file
    void saveFileOptionalDialog(bool forceDialogWindow = false);

    bool openFileInternal(const std::filesystem::path& path);
    bool saveFileInternal(const std::filesystem::path& path);
};
#endif // OPENFILEINFO_H
