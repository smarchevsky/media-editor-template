#ifndef WINDOW_H
#define WINDOW_H
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>

#include "graphics/gl_framebuffer.h"

#include <functional>
#include <glm/vec2.hpp>
#include <iostream>
#include <optional>
#include <unordered_map>

typedef void* SDL_GLContext;

static SDL_Keymod operator|(SDL_Keymod a, SDL_Keymod b) { return SDL_Keymod((int)a | (int)b); }
enum class MouseButton : uint8_t {
    Left = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right = SDL_BUTTON_RIGHT,
};

struct KeyWithModifier {
    KeyWithModifier(SDL_KeyCode key, SDL_Keymod mod, bool down)
        : key(key)
        , mod(mod)
        , down(down)
    {
    }
    bool operator==(const KeyWithModifier& rhs) const { return key == rhs.key && mod == rhs.mod && down == rhs.down; }
    SDL_KeyCode key = SDLK_UNKNOWN;
    uint16_t mod = KMOD_NONE;
    bool down = true;
};

namespace std {
template <>
struct hash<KeyWithModifier> {
    std::size_t operator()(const KeyWithModifier& k) const
    {
        uint64_t keyAction64
            = (uint64_t)k.key << 32
            | (uint64_t)((uint64_t)k.mod << 16)
            | (uint64_t)((uint64_t)k.down);
        return std::hash<uint64_t>()(static_cast<uint64_t>(keyAction64));
    }
};
}

enum class DragState {
    MouseUp,
    StartDrag,
    ContinueDrag
};

typedef std::function<void(glm::ivec2 currentPos, glm::ivec2 delta)> MouseMoveEvent;
typedef std::function<void(glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 delta, DragState)> MouseDragEvent;
typedef std::function<void(float scrollDelta, glm::ivec2 mousePos)> MouseScrollEvent;
typedef std::function<void(glm::ivec2 mousePos)> MouseDownEvent;
typedef std::function<void(glm::ivec2 mousePos)> MouseUpEvent;
typedef std::function<void(glm::ivec2 oldSize, glm::ivec2 newSize)> ScreenResizeEvent;
typedef std::function<void()> KeyEvent;
typedef std::function<void(KeyWithModifier)> AnyKeyEvent;
typedef std::function<void()> ImGuiContextFunctions;

class MouseEventData {
    MouseMoveEvent m_mouseMoveEvent {};
    MouseDragEvent m_mouseDragEvent {};

    MouseDownEvent m_mouseDownEvent {};
    MouseUpEvent m_mouseUpEvent {};

    glm::ivec2 m_startMouseDragPos {};
    DragState m_dragState = DragState::MouseUp;
    bool m_buttomPressed = false;

public:
    void runMouseMoveEvents(glm::ivec2 currentPos, glm::ivec2 delta)
    {
        if (m_mouseMoveEvent) {
            m_mouseMoveEvent(currentPos, delta);
        }
        if (m_mouseDragEvent && m_buttomPressed) {
            m_mouseDragEvent(m_startMouseDragPos, currentPos, delta, m_dragState);
        }
        if (m_dragState == DragState::StartDrag)
            m_dragState = DragState::ContinueDrag;
    }

    void setMouseMoveEvent(const MouseMoveEvent& event) { m_mouseMoveEvent = event; }
    void setMouseDragEvent(const MouseDragEvent& event) { m_mouseDragEvent = event; }

    void setMouseDownEvent(const MouseDownEvent& event) { m_mouseDownEvent = event; }
    void setMouseUpEvent(const MouseUpEvent& event) { m_mouseUpEvent = event; }

    void mouseDown(glm::ivec2 mousePos)
    {
        if (m_mouseDownEvent)
            m_mouseDownEvent(mousePos);

        m_buttomPressed = true;
        m_startMouseDragPos = mousePos;
        m_dragState = DragState::StartDrag;
    }

    void mouseUp(glm::ivec2 mousePos)
    {
        if (m_mouseUpEvent)
            m_mouseUpEvent(mousePos);

        m_buttomPressed = false;
        m_dragState = DragState::MouseUp;
    }

    // operator bool() { return !!m_event; }
};

// kinda window wrapper, you can wrap SDL window the same way

class Window : public GLFrameBufferBase {
public:
    Window(glm::ivec2 size, const std::string& name);
    Window(const Window&) = delete;
    Window(Window&& rhs) = default;
    ~Window();

    void setMouseDragEvent(MouseButton button, MouseDragEvent event);
    void setMouseMoveEvent(MouseButton button, MouseMoveEvent event);
    void setMouseDownEvent(MouseButton button, MouseDownEvent event);
    void setMouseScrollEvent(MouseScrollEvent event);

    void addKeyDownEvent(SDL_KeyCode key, SDL_Keymod mod, KeyEvent event);
    void addKeyUpEvent(SDL_KeyCode key, SDL_Keymod mod, KeyEvent event);

    void setAnyKeyDownOnceEvent(const std::string& reason, AnyKeyEvent event);
    void setAnyKeyReason(const std::string& reason);
    void setScreenResizeEvent(ScreenResizeEvent screenResizeEvent);

    void drawImGuiContext(ImGuiContextFunctions imguiFunctions);
    void display();
    void bind() const override;

    bool isOpen();
    void exit();
    void setTitle(const std::string& title);

    bool processEvents();

    glm::vec2 toCoordonates01(const glm::vec2 p) { return p / glm::vec2(m_windowSize); }
    glm::vec2 toCoordonatesM11(const glm::vec2 p) { return toCoordonates01(p) * 2.f - 1.f; }
    // glm::vec2 toOffsetM11(const glm::vec2 p) { return toCoordonates01(p) * 2.f; }
    glm::ivec2 getSize() { return m_windowSize; }

private:
    bool processEvent(const SDL_Event* event);

private: // SDL stuff
    class SDL_Window* m_SDLwindow {};
    SDL_GLContext m_GLcontext {};
    int m_SDLWindowID {};

private: // events
    MouseEventData* getMouseEventData(MouseButton button);
    MouseEventData m_mouseEventLMB {}, m_mouseEventMMB {}, m_mouseEventRMB {};
    MouseScrollEvent m_mouseScrollEvent {};
    std::unordered_map<KeyWithModifier, KeyEvent> m_keyMap;

    // allow any key event once
    std::unordered_map<std::string, AnyKeyEvent> m_anyKeyDownEvents;
    std::optional<std::string> m_anyKeyDownReason;

    ScreenResizeEvent m_screenResizeEvent;

    glm::ivec2 m_mousePos {};
    glm::ivec2 m_windowSize {};

    class ImFont* m_robotoFont;
    bool m_isOpen = true;

    static uint32_t s_instanceCounter;
};

#endif // WINDOW_H
