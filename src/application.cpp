#include "application.h"
#include "imgui_filesystem.h"

const char* shaderSource_defaultVertex =
    R"(\
#version 330 core
#define IDENTITY mat4(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1)

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;

out vec2 UV;

uniform mat4 matViewProjection = IDENTITY;
uniform mat4 matModel = IDENTITY;

void main()
{
   gl_Position = matViewProjection * vec4(inPos, 0.0, 1.0);
   UV = inUV;
}
)";

const char* shaderSource_defaultFragment =
    R"(\
#version 330 core

in vec2 UV;
out vec4 FragColor;

uniform sampler2D texture;

void main()
{
    // FragColor = vec4(UV, 0.0, 1.0);
    FragColor = texture2D(texture, UV);
}
)";

static const std::string strNoOpenFileFunction("Open function is not specified");
static const std::string strNoSaveFileFunction("Save function is not specified");
static const std::string strOpenFileFailed("Failed to open file: ");
static const std::string strSaveFileFailed("Failed to save file: ");
static const std::string strOpenFileSuccess("File opened successfully: ");
static const std::string strSaveFileSuccess("File saved successfully: ");

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);

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
    // m_window->clear();
}

void Application::init()
{
    // auto& mode = sf::VideoMode::getFullscreenModes()[0];
    // m_window.emplace(mode, "", sf::Style::Fullscreen);
    m_window.emplace(glm::ivec2(2000, 1000), "Fucking awesome application");
    m_quadShader.create(shaderSource_defaultVertex, shaderSource_defaultFragment, "default");
    m_camera.setViewportSize(m_window->getSize());

    m_texture.fromImage(Image(projectDir / "resources" / "UV_checker_Map_byValle.jpg"));
    m_quadShader.setUniform("texture", m_texture);

    addFileInteractionInfo("Primary", "png,jpg", nullptr, nullptr);

    // open file
    m_window->addKeyDownEvent(SDLK_o, KMOD_CTRL,
        std::bind(&Application::openFileDialog, this, "Primary"));

    // save file
    m_window->addKeyDownEvent(SDLK_s, KMOD_CTRL,
        std::bind(&Application::saveFileOptionalDialog, this, "Primary", false));

    // save file as
    m_window->addKeyDownEvent(SDLK_s, KMOD_CTRL | KMOD_SHIFT,
        std::bind(&Application::saveFileOptionalDialog, this, "Primary", true));

    m_window->setMouseScrollEvent( // zoom on scroll
        [this](float diff, glm::ivec2 mousePos) {
            float scaleFactor = pow(1.1f, -diff);

            m_camera.multiplyScaleOffseted(scaleFactor, mousePos);
        });

    m_window->setMouseDragEvent(MouseButton::Middle, // drag on MMB
        [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
            m_camera.addOffset_View(glm::vec2(-currentDelta));
        });

    m_window->setScreenResizeEvent([this](glm::ivec2 oldSize, glm::ivec2 newSize) {
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
    while (m_window && m_window->isOpen()) {
        m_window->processEvents();
        m_window->bind();
        m_quadShader.bind();

        glm::mat4 viewProjection;
        if (m_camera.getViewProjection(viewProjection))
            m_quadShader.setUniform("matViewProjection", viewProjection);

        drawContext();

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
