#include "application.h"
#include "graphics/gl_shader.h"
#include "imgui_filesystem.h"
#include <SDL2/SDL.h>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

Application::Application()
    : m_window(glm::ivec2(2000, 1000), "Fucking awesome application")
{
}

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

void Application::updateWindow(float dt)
{
    // m_window->clear();
}

void Application::init()
{
    // auto& mode = sf::VideoMode::getFullscreenModes()[0];
    // m_window.emplace(mode, "", sf::Style::Fullscreen);

    m_camera.setViewportSize(m_window.getSize());

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

    m_window.setMouseScrollEvent( // zoom on scroll
        [this](float diff, glm::ivec2 mousePos) {
            float scaleFactor = pow(1.1f, -diff);

            m_camera.multiplyScaleOffseted(scaleFactor, mousePos);
        });

    m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
        [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
            m_camera.addOffset_View(glm::vec2(-currentDelta));
        });

    m_window.setScreenResizeEvent([this](glm::ivec2 oldSize, glm::ivec2 newSize) {
        m_camera.setViewportSize(glm::vec2(newSize));
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
    m_currentTimeStamp = SDL_GetPerformanceCounter();

    while (m_window.isOpen()) {
        uint64_t timeNow = SDL_GetPerformanceCounter();
        float dt = (float)((timeNow - m_currentTimeStamp) / (double)SDL_GetPerformanceFrequency());
        // dt = std::clamp(dt, 1 / 500.f, 1 / 10.f);
        dt = dt > 0 ? dt : 1 / 60.f;
        m_currentTimeStamp = timeNow;

        m_window.processEvents();

        glm::mat4 viewProjection;
        if (m_camera.getViewProjection(viewProjection)) {
            GLShaderManager::get().getDefaultShader2d()->setUniform(
                "matViewProjection", viewProjection, UniformType::Mat4, 0);
        }
        // m_window.bind();
        updateWindow(dt);

        m_window.display();
    }
}

void Application::printWarningMessage(const std::string& msg) { LOG(msg); }
void Application::printNotificationMessage(const std::string& msg)
{
    m_window.setTitle(msg);
}

Application::~Application()
{
}
