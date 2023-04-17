#include "window.h"

#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_opengl.h>

// #include "imgui/imgui.h"
// #include "imgui/imgui_internal.h"
#include <cassert>
#include <filesystem>

#define MAX_DIRTY(x) m_showDisplayDirtyLevel = m_showDisplayDirtyLevel > x ? m_showDisplayDirtyLevel : x
namespace fs = std::filesystem;

uint32_t Window::s_instanceCounter = 0;
void Window::init()
{
}

MouseEventData* Window::getMouseEventData(MouseButton button)
{
    switch (button) {
    case MouseButton::Left:
        return &m_mouseEventLMB;
    case MouseButton::Middle:
        return &m_mouseEventMMB;
    case MouseButton::Right:
        return &m_mouseEventRMB;
    default: {
        return nullptr;
    }
    }
}

Window::Window(glm::ivec2 size, const std::string& name)
{
    m_windowSize = size;

    m_SDLWindow = SDL_CreateWindow(name.c_str(),
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        m_windowSize.x, m_windowSize.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    assert(m_SDLWindow && "Could not create window");

    gl_context = SDL_GL_CreateContext(m_SDLWindow);
    assert(gl_context && "Could not create GL context");

    m_renderer = SDL_CreateRenderer(m_SDLWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(m_renderer && "Could not create renderer");

    m_SDLWindowID = SDL_GetWindowID(m_SDLWindow);
    assert(m_SDLWindowID && "Invalid window ID");

    SDL_GL_MakeCurrent(m_SDLWindow, gl_context);
}

Window::~Window()
{
}

bool Window::processEvents()
{
    SDL_Event event;
    bool somethingHappened = false;
    while (SDL_PollEvent(&event)) {
        if (event.window.windowID == m_SDLWindowID)
            somethingHappened |= processEvent(&event);
    }
    return somethingHappened;
}

bool Window::processEvent(const SDL_Event* event)
{
    bool ImGuiWantsCaptureMouse = false;
    // ImGuiIO& io = ImGui::GetIO();
    //  ImGuiWantsCaptureMouse = io.WantCaptureMouse;

    switch (event->type) {
    case SDL_QUIT: {
        exit();
        // p_engine->stop();
    } break;
    case SDL_MOUSEMOTION: {
        m_mousePos = glm::ivec2(event->motion.x, event->motion.y);
        glm::ivec2 relativeOffset(event->motion.xrel, event->motion.yrel);
        m_mouseEventLMB.runMouseMoveEvents(m_mousePos, relativeOffset);
        m_mouseEventMMB.runMouseMoveEvents(m_mousePos, relativeOffset);
        m_mouseEventRMB.runMouseMoveEvents(m_mousePos, relativeOffset);
        return true;
    }

    case SDL_MOUSEWHEEL: {
        if (!ImGuiWantsCaptureMouse) {
            if (m_mouseScrollEvent)
                m_mouseScrollEvent(event->wheel.preciseX, m_mousePos);
        }
        return true;
    }

    case SDL_MOUSEBUTTONDOWN: {
        if (!ImGuiWantsCaptureMouse) {
            auto mouseEventData = getMouseEventData((MouseButton)event->button.button);
            if (mouseEventData)
                mouseEventData->mouseDown(m_mousePos);
        }
        return true;
    }

    case SDL_MOUSEBUTTONUP: {
        if (!ImGuiWantsCaptureMouse) {
            auto mouseEventData = getMouseEventData((MouseButton)event->button.button);
            if (mouseEventData)
                mouseEventData->mouseUp(m_mousePos);
        }
        return true;
    }

    case SDL_WINDOWEVENT_FOCUS_GAINED: {
        // io.WantCaptureMouse = true;
        return true;
    };

    case SDL_TEXTINPUT: {
        // io.AddInputCharactersUTF8(event->text.text);
        return true;
    }

    case SDL_KEYDOWN: {
        {
            if (!ImGuiWantsCaptureMouse) {
                if (!event->key.repeat) {
                    KeyWithModifier keyMod(
                        (SDL_KeyCode)event->key.keysym.sym,
                        (SDL_Keymod)event->key.keysym.mod, true);

                    if (keyMod.key == SDL_KeyCode::SDLK_q && (keyMod.mod & SDL_Keymod::KMOD_CTRL)) {
                        exit();
                    } else {

                        if (!m_anyKeyDownReason) {
                            auto keyEventIter = m_keyMap.find(keyMod);
                            if (keyEventIter != m_keyMap.end())
                                keyEventIter->second();
                        } else {
                            const auto& foundEvent = m_anyKeyDownEvents.find(*m_anyKeyDownReason);
                            if (foundEvent != m_anyKeyDownEvents.end())
                                foundEvent->second(keyMod);
                            m_anyKeyDownReason = {};
                        }
                    }
                }
            }
        }
        return true;
    }

    case SDL_KEYUP: {
        if (!ImGuiWantsCaptureMouse) {
            KeyWithModifier keyMod(
                (SDL_KeyCode)event->key.keysym.sym,
                (SDL_Keymod)event->key.keysym.mod, false);

            if (!m_anyKeyDownReason) {
                auto keyEventIter = m_keyMap.find(keyMod);
                if (keyEventIter != m_keyMap.end())
                    keyEventIter->second();
            }
        }
        return true;
    }

    case SDL_WINDOWEVENT: {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            glm::ivec2 oldScreenSize = m_windowSize;
            m_windowSize = glm::ivec2(event->window.data1, event->window.data2);
            //        m_windowSize = newSize;
            //        applyScaleAndOffset();
            //        glm::ivec2 offset = m_windowSize - oldScreenSize;
            //        if (offset.x != 0) {
            //            if (ImGuiContext* g = ImGui::GetCurrentContext()) {
            //                for (auto& w : g->Windows) {
            //                    glm::vec2 windowHalfSize(w->Size.x * 0.5f, w->Size.y * 0.5f);
            //                    if (w->Pos.x + windowHalfSize.x > oldScreenSize.x / 2.f) {
            //                        w->Pos.x += offset.x;
            //                        if (w->Pos.x + windowHalfSize.x < newSize.x / 2.f)
            //                            w->Pos.x = newSize.x / 2.f - windowHalfSize.x;
            //                    }
            //                }
            //            }
            //        }
            if (m_screenResizeEvent)
                m_screenResizeEvent(oldScreenSize, m_windowSize);
            SDL_RenderPresent(m_renderer);
            break;
        }
        } // switch window event

        return true;
    }
    } // end switch
    return false;
}

void Window::setMouseDragEvent(MouseButton button, MouseDragEvent event)
{
    auto mouseEventData = getMouseEventData(button);
    if (mouseEventData) {
        mouseEventData->setMouseDragEvent(event);
    }
}

void Window::setMouseMoveEvent(MouseButton button, MouseMoveEvent event)
{
    auto mouseEventData = getMouseEventData(button);
    if (mouseEventData) {
        mouseEventData->setMouseMoveEvent(event);
    }
}

void Window::setMouseDownEvent(MouseButton button, MouseDownEvent event)
{
    auto mouseEventData = getMouseEventData(button);
    if (mouseEventData) {
        mouseEventData->setMouseDownEvent(event);
    }
}

void Window::setMouseScrollEvent(MouseScrollEvent event) { m_mouseScrollEvent = event; }

void Window::addKeyDownEvent(SDL_KeyCode key, SDL_Keymod mod, KeyEvent event)
{
    m_keyMap.insert({ KeyWithModifier(key, mod, true), event });
}

void Window::addKeyUpEvent(SDL_KeyCode key, SDL_Keymod mod, KeyEvent event)
{
    m_keyMap.insert({ KeyWithModifier(key, mod, false), event });
}

void Window::setAnyKeyDownOnceEvent(const std::string& reason, AnyKeyEvent event) { m_anyKeyDownEvents[reason] = event; }

void Window::setAnyKeyReason(const std::string& reason) { m_anyKeyDownReason = reason; }

void Window::setScreenResizeEvent(ScreenResizeEvent screenResizeEvent) { m_screenResizeEvent = screenResizeEvent; }

void Window::drawImGuiContext(ImGuiContextFunctions imguiFunctions)
{
    // ImGui::SFML::Update(*this, m_deltaClock.restart());
    // ImGui::PushFont(m_robotoFont);
    // if (imguiFunctions)
    //     imguiFunctions();
    // ImGui::PopFont();
    // ImGui::SFML::Render(*this);
}

bool Window::isOpen() { return m_isOpen; }

void Window::exit() { m_isOpen = false; }

void Window::setTitle(const std::string& title)
{
    SDL_SetWindowTitle(m_SDLWindow, title.c_str());
}

void Window::setScale(float scale) { m_scale = scale, applyScaleAndOffset(); }

void Window::addScale(float scaleFactor) { m_scale *= scaleFactor, applyScaleAndOffset(); }

float Window::getScale() const { return m_scale; }

void Window::addOffset(glm::vec2 offset) { m_viewOffset += offset, applyScaleAndOffset(); }

void Window::setOffset(glm::vec2 offset) { m_viewOffset = offset; }

glm::vec2 Window::getOffset() const { return m_viewOffset; }

void Window::applyScaleAndOffset()
{
    // sf::View view(m_viewOffset, toFloat(m_windowSize) * m_scale);
    // setView(view);
}

void Window::display()
{
    SDL_GL_SwapWindow(m_SDLWindow);
}
