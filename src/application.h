#ifndef APP_ABSTRACT_H
#define APP_ABSTRACT_H

#include "window.h"
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace ImguiUtils {
class FileSystemNavigator;
}
class Window;

typedef std::function<bool(const std::filesystem::path&)> FileInteractionFunction;

class Application {
    struct OpenFileData {
        explicit OpenFileData(
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

    std::map<std::string, OpenFileData> m_openFileData;

protected:
    std::optional<Window> m_window;
    std::unique_ptr<ImguiUtils::FileSystemNavigator> m_fsNavigator;
    void addFileInteractionInfo(const std::string& name, const std::string& supportedExtensions,
        FileInteractionFunction fileReader, FileInteractionFunction fileWriter);

protected:
    void openFileDialog(const std::string& openFileName);
    // if m_openedFileName (and !forceDialogWindow) force save current file
    void saveFileOptionalDialog(const std::string& openFileDataName, bool forceDialogWindow = false);

    virtual void drawContext();
    virtual void drawImGuiLayer();
    virtual void printWarningMessage(const std::string& msg);
    virtual void printNotificationMessage(const std::string& msg);

public:
    Application();
    virtual void init();
    virtual ~Application();

    void mainLoop();
};

#endif // APP_ABSTRACT_H
