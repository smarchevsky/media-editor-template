#ifndef APP_ABSTRACT_H
#define APP_ABSTRACT_H

#include "helper_general.h"
#include "openfileinfo.h"
#include "window.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace ImguiUtils {
class FileSystemNavigator;
}
class Window;

class Application : NoCopy<Application> {
protected:
    Window m_window;
    std::map<std::string, OpenFileInfo> m_openFileData;
    std::unique_ptr<ImguiUtils::FileSystemNavigator> m_fsNavigator;

protected:
    void addFileInteractionInfo(const std::string& name, const std::string& supportedExtensions,
        FileInteractionFunction fileReader, FileInteractionFunction fileWriter);

public:
    void openFileDialog(const std::string& openFileName);
    void saveFileOptionalDialog(const std::string& openFileDataName, bool forceDialogWindow = false);

    virtual void updateWindow(float dt) { }
    virtual void drawImGuiLayer();
    virtual void printWarningMessage(const std::string& msg);
    virtual void printNotificationMessage(const std::string& msg);

public:
    Application();
    virtual void init();
    virtual ~Application();

    void mainLoop();

private:
    friend class OpenFileInfo;
    uint64_t m_currentTimeStamp {};
};

#endif // APP_ABSTRACT_H
