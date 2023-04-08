#include "window.h"

#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <filesystem>

#define MAX_DIRTY(x) m_showDisplayDirtyLevel = m_showDisplayDirtyLevel > x ? m_showDisplayDirtyLevel : x
namespace fs = std::filesystem;

uint32_t Window::s_instanceCounter = 0;
void Window::init()
{
    bool s = ImGui::SFML::Init(*this, false);
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->ClearFonts();
    m_robotoFont = io.Fonts->AddFontFromFileTTF((fs::path(FONT_DIR) / "Roboto.ttf").c_str(), 18.0f);

    s |= ImGui::SFML::UpdateFontTexture();

    s = s;
    s_instanceCounter++;
}

Window::~Window()
{
    s_instanceCounter--;
    if (s_instanceCounter == 0)
        ImGui::SFML::Shutdown();
}

void Window::processEvents()
{
    sf::Event event;
    ImGui::SFML::SetCurrentWindow(*this);
    if (auto* imContext = ImGui::GetCurrentContext()) {
        float fadeRatio = imContext->DimBgRatio;
        if (fadeRatio > 0 && fadeRatio < 1)
            MAX_DIRTY(10);
        else if (imContext->LastActiveIdTimer == 0.f)
            MAX_DIRTY(2);
    }

    while (pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        ImGuiIO& io = ImGui::GetIO();

        switch (event.type) {
        case sf::Event::Closed:
            exit();
            break;

        case sf::Event::Resized: {
            auto oldScreenSize = m_windowSize;
            const auto& newSize = toUInt(event.size);
            m_windowSize = newSize;
            applyScaleAndOffset();

            ivec2 offset = toInt(m_windowSize - oldScreenSize);

            if (offset.x != 0) {
                if (ImGuiContext* g = ImGui::GetCurrentContext()) {
                    for (auto& w : g->Windows) {
                        vec2 windowHalfSize(w->Size.x * 0.5f, w->Size.y * 0.5f);
                        if (w->Pos.x + windowHalfSize.x > oldScreenSize.x / 2.f) {
                            w->Pos.x += offset.x;
                            if (w->Pos.x + windowHalfSize.x < newSize.x / 2.f)
                                w->Pos.x = newSize.x / 2.f - windowHalfSize.x;
                        }
                    }
                }
            }

            if (m_screenResizeEvent)
                m_screenResizeEvent(oldScreenSize, m_windowSize);
        } break;

        case sf::Event::LostFocus:
            break;

        case sf::Event::GainedFocus:
            io.WantCaptureMouse = true;
            break;

        case sf::Event::TextEntered:
            break;

        case sf::Event::KeyPressed: {
            if (event.key.control && event.key.code == sf::Keyboard::Key::Q)
                exit();

            else if (!io.WantCaptureKeyboard) {
                KeyWithModifier currentKey(event.key.code,
                    makeModifier(
                        event.key.alt,
                        event.key.control,
                        event.key.shift,
                        event.key.system),
                    true);

                if (m_anyKeyDownReason) {
                    const auto& foundEvent = m_anyKeyDownEvents.find(*m_anyKeyDownReason);
                    if (foundEvent != m_anyKeyDownEvents.end()) {
                        foundEvent->second(currentKey);
                    }
                    m_anyKeyDownReason = {};
                } else {
                    auto keyEventIter = m_keyMap.find(currentKey);
                    if (keyEventIter != m_keyMap.end())
                        keyEventIter->second();
                }
            }
        } break;

        case sf::Event::KeyReleased: {
            if (!io.WantCaptureKeyboard) {
                KeyWithModifier currentKey(event.key.code,
                    makeModifier(
                        event.key.alt,
                        event.key.control,
                        event.key.shift,
                        event.key.system),
                    false);

                auto keyEventIter = m_keyMap.find(currentKey);
                if (keyEventIter != m_keyMap.end())
                    keyEventIter->second();
            }
        } break;

        case sf::Event::MouseWheelScrolled: {
            if (!io.WantCaptureMouse) {
                if (m_mouseScrollEvent)
                    m_mouseScrollEvent(event.mouseWheelScroll.delta, m_mousePos);
            } else {
            }

        } break;

        case sf::Event::MouseButtonPressed: {
            if (!io.WantCaptureMouse) {
                auto mouseEventData = getMouseEventData(event.mouseButton.button);
                if (mouseEventData)
                    mouseEventData->mouseDown(m_mousePos, true);
            }
        } break;

        case sf::Event::MouseButtonReleased: {
            if (!io.WantCaptureMouse) {
                auto mouseEventData = getMouseEventData(event.mouseButton.button);
                if (mouseEventData)
                    mouseEventData->mouseDown(m_mousePos, false);
            }
        } break;

        case sf::Event::MouseMoved: {
            ivec2 prevPos = m_mousePos;
            m_mousePos = toInt(event.mouseMove);
            m_mouseEventLMB.runMouseMoveEvents(m_mousePos, m_mousePos - prevPos);
            m_mouseEventMMB.runMouseMoveEvents(m_mousePos, m_mousePos - prevPos);
            m_mouseEventRMB.runMouseMoveEvents(m_mousePos, m_mousePos - prevPos);
        } break;

        case sf::Event::MouseEntered:
            break;

        case sf::Event::MouseLeft:
            break;

        default: {
        }
        }
        MAX_DIRTY(2);
    }
}

void Window::setMouseDragEvent(sf::Mouse::Button button, MouseDragEvent event)
{
    auto mouseEventData = getMouseEventData(button);
    if (mouseEventData) {
        mouseEventData->setMouseDragEvent(event);
    }
}

void Window::setMouseMoveEvent(sf::Mouse::Button button, MouseMoveEvent event)
{
    auto mouseEventData = getMouseEventData(button);
    if (mouseEventData) {
        mouseEventData->setMouseMoveEvent(event);
    }
}

void Window::setMouseDownEvent(sf::Mouse::Button button, MouseDownEvent event)
{
    auto mouseEventData = getMouseEventData(button);
    if (mouseEventData) {
        mouseEventData->setMouseDownEvent(event);
    }
}

void Window::addKeyDownEvent(sf::Keyboard::Key key, ModifierKey modifier, KeyEvent event)
{
    m_keyMap.insert({ KeyWithModifier(key, modifier, true), event });
}

void Window::addKeyUpEvent(sf::Keyboard::Key key, ModifierKey modifier, KeyEvent event)
{
    m_keyMap.insert({ KeyWithModifier(key, modifier, false), event });
}

void Window::drawImGuiContext(ImGuiContextFunctions imguiFunctions)
{
    ImGui::SFML::Update(*this, m_deltaClock.restart());
    ImGui::PushFont(m_robotoFont);
    if (imguiFunctions)
        imguiFunctions();
    ImGui::PopFont();
    ImGui::SFML::Render(*this);
}

void Window::display()
{
    if (m_showDisplayDirtyLevel > 0) {
        m_showDisplayDirtyLevel--;
    } else if (m_showDisplayDirtyLevel < 0) {
        m_showDisplayDirtyLevel = 0;
    }
    sf::Window::display();
}

void Window::exit()
{
    close();
}
