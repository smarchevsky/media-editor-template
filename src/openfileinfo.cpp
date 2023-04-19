#include "openfileinfo.h"
#include "application.h"
#include "imgui_filesystem.h"

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

static const std::string strNoOpenFileFunction("Open function is not specified");
static const std::string strNoSaveFileFunction("Save function is not specified");
static const std::string strOpenFileFailed("Failed to open file: ");
static const std::string strSaveFileFailed("Failed to save file: ");
static const std::string strOpenFileSuccess("File opened successfully: ");
static const std::string strSaveFileSuccess("File saved successfully: ");

bool OpenFileInfo::openFileInternal(const fs::path& fullFilePath)
{
    bool success = false;
    if (m_openFileFunction) {
        if (m_openFileFunction(fullFilePath)) {
            success = true;
            m_currentFileName = fullFilePath.filename();
            m_currentDir = fullFilePath.relative_path();

            m_parentApp->printNotificationMessage(strOpenFileSuccess);
        } else {
            m_parentApp->printWarningMessage(strOpenFileFailed);
        }
    } else {
        m_parentApp->printWarningMessage(strNoOpenFileFunction);
    }
    return success;
}

bool OpenFileInfo::saveFileInternal(const fs::path& fullFilePath)
{
    bool success = false;
    if (m_saveFileFunction) {
        if (m_saveFileFunction(fullFilePath)) {
            success = true;
            m_currentFileName = fullFilePath.filename();
            m_currentDir = fullFilePath.relative_path();

            m_parentApp->printNotificationMessage(strSaveFileSuccess);
        } else {
            m_parentApp->printWarningMessage(strSaveFileFailed);
        }
    } else {
        m_parentApp->printWarningMessage(strNoSaveFileFunction);
    }
    return success;
}

OpenFileInfo::OpenFileInfo(Application* const parentApp,
    const std::string& extensions,
    const fs::path& currentDir,
    FileInteractionFunction fileReader,
    FileInteractionFunction fileWriter)
    : m_parentApp(parentApp)
    , m_supportedFileExtensions(extensions)
    , m_currentDir(currentDir)
    , m_openFileFunction(fileReader)
    , m_saveFileFunction(fileWriter)
{
}

void OpenFileInfo::openFileDialog()
{
    m_parentApp->m_fsNavigator.reset(new ImguiUtils::FileReader(
        "Open file", m_currentDir, m_supportedFileExtensions,
        [this](const std::filesystem::path& fullPath) {
            return this->openFileInternal(fullPath);
        }));
}

void OpenFileInfo::saveFileOptionalDialog(bool forceDialogWindow)
{
    if (m_currentFileName && !forceDialogWindow) {
        saveFileInternal(m_currentDir / *m_currentFileName);
    } else {
        m_parentApp->m_fsNavigator.reset(new ImguiUtils::FileWriter(
            "Save file", m_currentDir, m_supportedFileExtensions,
            [this](const std::filesystem::path& fullPath) {
                return this->saveFileInternal(fullPath);
            }));
    }
}
