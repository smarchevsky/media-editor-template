#include "application.h"
#include "imgui_filesystem.h"

#ifndef LOG
#define LOG(x) std::cout << x << std::endl
#endif

static const std::string strNoOpenFileFunction("Open function is not specified");
static const std::string strNoSaveFileFunction("Save function is not specified");
static const std::string strOpenFileFailed("Failed to open file: ");
static const std::string strSaveFileFailed("Failed to save file: ");
static const std::string strOpenFileSuccess("File opened successfully: ");
static const std::string strSaveFileSuccess("File saved successfully: ");

namespace fs = std::filesystem;

bool Application::OpenFileData::openFileInternal(const fs::path& fullFilePath)
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

bool Application::OpenFileData::saveFileInternal(const fs::path& fullFilePath)
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

Application::OpenFileData::OpenFileData(Application* const parentApp,
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

void Application::OpenFileData::openFileDialog()
{
    m_parentApp->m_fsNavigator.reset(new ImguiUtils::FileReader(
        "Open file", m_currentDir, m_supportedFileExtensions,
        [this](const std::filesystem::path& fullPath) {
            return this->openFileInternal(fullPath);
        }));
}

void Application::OpenFileData::saveFileOptionalDialog(bool forceDialogWindow)
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

Application::Application()
{
    m_window.emplace(sf::VideoMode(1024, 768), "");
    // m_primaryOpenFileData.currentDir = HOME_DIR;
}

void Application::addFileInteractionInfo(const std::string& name, const std::string& supportedExtensions,
    FileInteractionFunction fileReader, FileInteractionFunction fileWriter)
{
    fs::path primaryDir = HOME_DIR;
    m_openFileData.insert({ name, OpenFileData(this, supportedExtensions, primaryDir, fileReader, fileWriter) });
}

void Application::openFileDialog(const std::string& openFileDataName)
{
    const auto it = m_openFileData.find(openFileDataName);
    if (it != m_openFileData.end()) {
        auto& fileData = it->second;
        fileData.openFileDialog();

    } else
        printWarningMessage("Invalid open file data name");
}

void Application::saveFileOptionalDialog(const std::string& openFileDataName, bool forceDialogWindow)
{
    const auto it = m_openFileData.find(openFileDataName);
    if (it != m_openFileData.end()) {
        auto& fileData = it->second;
        fileData.saveFileOptionalDialog(forceDialogWindow);
    } else
        printWarningMessage("Invalid open file data name");
}

void Application::drawContext()
{
    m_window->clear();
}

void Application::init()
{
    addFileInteractionInfo("Primary", "png,jpg", nullptr, nullptr);

    // open file
    m_window->addKeyDownEvent(sf::Keyboard::O, ModifierKey::Control,
        std::bind(&Application::openFileDialog, this, "Primary"));

    // save file
    m_window->addKeyDownEvent(sf::Keyboard::S, ModifierKey::Control,
        std::bind(&Application::saveFileOptionalDialog, this, "Primary", false));

    // save file as
    m_window->addKeyDownEvent(sf::Keyboard::S, ModifierKey::Control | ModifierKey::Shift,
        std::bind(&Application::saveFileOptionalDialog, this, "Primary", true));

    m_window->setMouseScrollEvent( // zoom on scroll
        [this](float diff, ivec2 mousePos) {
            float scaleFactor = pow(1.1f, -diff);
            m_window->addScale(scaleFactor);
            vec2 mouseWorld = m_window->mapPixelToCoords(mousePos);
            vec2 offset = (mouseWorld - m_window->getOffset()) * log(scaleFactor);
            m_window->addOffset(-offset);
        });

    m_window->setMouseDragEvent(sf::Mouse::Middle, // drag on MMB
        [this](ivec2 startPos, ivec2 currentPos, ivec2 currentDelta, DragState dragState) {
            m_window->addOffset(toFloat(-currentDelta) * m_window->getScale());
        });
}

void Application::drawImGuiLayer()
{
    if (m_fsNavigator) { // file reader/saver here
        if (!m_fsNavigator->showInImGUI()) {
            m_fsNavigator.reset();
        }
    }
}

void Application::mainLoop()
{
    while (m_window && m_window->isOpen()) {
        m_window->processEvents();

        if (true || m_window->windowMayBeDirty()) {
            drawContext();

            m_window->drawImGuiContext(std::bind(&Application::drawImGuiLayer, this));
        }

        m_window->display();
    }
}

void Application::printWarningMessage(const std::string& msg) { LOG(msg); }
void Application::printNotificationMessage(const std::string& msg)
{
    if (m_window) {
        m_window->setTitle(msg);
    }
}

Application::~Application()
{
}
