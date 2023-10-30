#include "application.h"
#include "imgui_filesystem.h"
#include <SDL2/SDL.h>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

Application::Application()
    : m_window(glm::ivec2(2000, 1000), "Media editor template application")
{
}

Application::~Application() { }

void Application::addFileInteractionInfo(const std::string& name, const std::string& supportedExtensions,
    FileInteractionFunction fileReader, FileInteractionFunction fileWriter)
{
    fs::path primaryDir = HOME_DIR;
    m_openFileData.insert({ name, OpenFileInfo(this, supportedExtensions, primaryDir, fileReader, fileWriter) });
}

void Application::openFileDialog(const std::string& openFileDataName)
{
    const auto it = m_openFileData.find(openFileDataName);
    if (it != m_openFileData.end()) {
        auto& fileData = it->second;
        fileData.openFileDialog();

    } else
        printWarningMessage("No open info");
}

void Application::saveFileOptionalDialog(const std::string& openFileDataName, bool forceDialogWindow)
{
    const auto it = m_openFileData.find(openFileDataName);
    if (it != m_openFileData.end()) {
        auto& fileData = it->second;
        fileData.saveFileOptionalDialog(forceDialogWindow);
    } else
        printWarningMessage("No save info");
}

void Application::init()
{
    addFileInteractionInfo("Primary", "png,jpg", nullptr, nullptr);

    // open file
    m_window.addKeyDownEvent(SDLK_o, KMOD_CTRL,
        std::bind(&Application::openFileDialog, this, "Primary"));

    // save file
    m_window.addKeyDownEvent(SDLK_s, KMOD_CTRL,
        std::bind(&Application::saveFileOptionalDialog, this, "Primary", false));

    // save file as
    m_window.addKeyDownEvent(SDLK_s, KMOD_CTRL | KMOD_SHIFT,
        std::bind(&Application::saveFileOptionalDialog, this, "Primary", true));
}

void Application::mainLoop()
{
    m_currentTimeStamp = SDL_GetPerformanceCounter();

    while (m_window.isOpen()) {

        uint64_t timeNow = SDL_GetPerformanceCounter();
        float dt = (float)((timeNow - m_currentTimeStamp) / (double)SDL_GetPerformanceFrequency());
        dt = dt > 0 ? dt : 1 / 60.f;
        m_currentTimeStamp = timeNow;

        m_window.processEvents();

        m_window.preDrawImGui();
        updateWindow(dt);
        if (m_fsNavigator && !m_fsNavigator->showInImGUI())
            m_fsNavigator.reset();

        m_window.postDrawImGui();

        m_window.display();
    }
}

void Application::printWarningMessage(const std::string& msg) const { LOG(msg); }
void Application::printNotificationMessage(const std::string& msg) const
{
    // m_window.setTitle(msg);
}
