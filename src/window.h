#ifndef WINDOW_H
#define WINDOW_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vec2.h>

#ifndef LOG
#define LOG(x) std::cout << x << std::endl
#endif

enum class ModifierKey : uint8_t {
    None = 0,
    Alt = 1,
    Control = 1 << 1,
    Shift = 1 << 2,
    System = 1 << 3
};

static ModifierKey operator|(ModifierKey a, ModifierKey b) { return ModifierKey((int)a | (int)b); }
static ModifierKey makeModifier(bool alt = false, bool ctrl = false, bool shift = false, bool system = false)
{
    return ModifierKey(alt | ctrl << 1 | shift << 2 | system << 3);
}
struct KeyWithModifier {
    KeyWithModifier(sf::Keyboard::Key key, ModifierKey mod, bool down)
        : key(key)
        , mod(mod)
        , down(down)
    {
    }
    bool operator==(const KeyWithModifier& rhs) const { return key == rhs.key && mod == rhs.mod && down == rhs.down; }

    sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;
    ModifierKey mod = ModifierKey::None;
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

typedef std::function<void(ivec2 currentPos, ivec2 delta)> MouseMoveEvent;
typedef std::function<void(ivec2 startPos, ivec2 currentPos, ivec2 delta, DragState)> MouseDragEvent;
typedef std::function<void(float scrollDelta, ivec2 mousePos)> MouseScrollEvent;
typedef std::function<void(ivec2 mousePos, bool mouseDown)> MouseDownEvent;
typedef std::function<void(uvec2 oldSize, uvec2 newSize)> ScreenResizeEvent;
typedef std::function<void()> KeyEvent;
typedef std::function<void(KeyWithModifier)> AnyKeyEvent;
typedef std::function<void()> ImGuiContextFunctions;

class MouseEventData {
    MouseMoveEvent m_mouseMoveEvent {};
    MouseDragEvent m_mouseDragEvent {};
    MouseDownEvent m_mouseDownEvent {};
    ivec2 m_startMouseDragPos {};
    DragState m_dragState = DragState::MouseUp;
    bool m_buttomPressed = false;

public:
    void runMouseMoveEvents(ivec2 currentPos, ivec2 delta)
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

    void mouseDown(ivec2 mousePos, bool down)
    {
        if (down) {
            m_buttomPressed = true;
            m_startMouseDragPos = mousePos;
            m_dragState = DragState::StartDrag;
        } else {
            m_buttomPressed = false;
            m_dragState = DragState::MouseUp;
        }
        if (m_mouseDownEvent)
            m_mouseDownEvent(mousePos, down);
    }

    // operator bool() { return !!m_event; }
};

// kinda window wrapper, you can wrap SDL window the same way

class Window : public sf::RenderWindow {
public:
    template <typename... Args>
    Window(Args&&... args)
        : sf::RenderWindow(std::forward<Args>(args)...)
        , m_windowSize(getSize())
        , m_viewOffset(toFloat(m_windowSize) / 2)
    {
        init();
        setVerticalSyncEnabled(true);
        setFramerateLimit(100);
    }
    ~Window();

    void processEvents();

    void setScale(float scale) { m_scale = scale, applyScaleAndOffset(); }
    void addScale(float scaleFactor) { m_scale *= scaleFactor, applyScaleAndOffset(); }
    float getScale() const { return m_scale; }
    void addOffset(vec2 offset) { m_viewOffset += offset, applyScaleAndOffset(); }
    void setOffset(vec2 offset) { m_viewOffset = offset; }
    vec2 getOffset() const { return m_viewOffset; }
    void applyScaleAndOffset()
    {
        sf::View view(m_viewOffset, toFloat(m_windowSize) * m_scale);
        setView(view);
    }

    void setMouseDragEvent(sf::Mouse::Button button, MouseDragEvent event);
    void setMouseMoveEvent(sf::Mouse::Button button, MouseMoveEvent event);
    void setMouseDownEvent(sf::Mouse::Button button, MouseDownEvent event);
    void setMouseScrollEvent(MouseScrollEvent event) { m_mouseScrollEvent = event; }

    void addKeyDownEvent(sf::Keyboard::Key key, ModifierKey modifier, KeyEvent event);
    void addKeyUpEvent(sf::Keyboard::Key key, ModifierKey modifier, KeyEvent event);

    void setAnyKeyDownOnceEvent(const std::string& reason, AnyKeyEvent event) { m_anyKeyDownEvents[reason] = event; }
    void setAnyKeyReason(const std::string& reason) { m_anyKeyDownReason = reason; }
    void setScreenResizeEvent(ScreenResizeEvent screenResizeEvent) { m_screenResizeEvent = screenResizeEvent; }

    void drawImGuiContext(ImGuiContextFunctions imguiFunctions);
    void display();

    bool windowMayBeDirty() { return !!m_showDisplayDirtyLevel; }
    void exit();

private:
    void init();
    MouseEventData* getMouseEventData(sf::Mouse::Button button)
    {
        switch (button) {
        case sf::Mouse::Left:
            return &m_mouseEventLMB;
        case sf::Mouse::Middle:
            return &m_mouseEventMMB;
        case sf::Mouse::Right:
            return &m_mouseEventRMB;
        default: {
            return nullptr;
        }
        }
    }

    MouseEventData m_mouseEventLMB {}, m_mouseEventMMB {}, m_mouseEventRMB {};
    MouseScrollEvent m_mouseScrollEvent {};
    std::unordered_map<KeyWithModifier, KeyEvent> m_keyMap;

    // allow any key event once
    std::unordered_map<std::string, AnyKeyEvent> m_anyKeyDownEvents;
    std::optional<std::string> m_anyKeyDownReason;

    ScreenResizeEvent m_screenResizeEvent;

    ivec2 m_mousePos {};
    uvec2 m_windowSize {};
    float m_scale = 1.f;
    vec2 m_viewOffset {};

    sf::Clock m_deltaClock;

    class ImFont* m_robotoFont;

    int m_showDisplayDirtyLevel = 10;
    static uint32_t s_instanceCounter;
};

#endif // WINDOW_H
