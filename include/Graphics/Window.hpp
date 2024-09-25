#pragma once

#include "Dependencies.hpp"
#include "Math/Vector.hpp"

CLASS_DECLARE(GraphicsAPI);

struct KeyEvent
{
    int Code;
    bool Down;
};

struct MouseEvent
{
    int Button;
    bool Down;
};

struct CursorEvent
{
    Ivec2 Pos;
};

struct ScrollEvent
{
    Ivec2 Offset;
};

struct WindowEvent
{
    union
    {
        KeyEvent KE;
        MouseEvent ME;
        CursorEvent CE;
        ScrollEvent SE;
    };

    enum EventType
    {
        KEY_EVENT,
        MOUSE_EVENT,
        CURSOR_EVENT,
        SCROLL_EVENT
    } Type;
};

class DisplayWindow
{
public:

    DisplayWindow(GraphicsAPI const& api, int width, int height, char const* title);

    ~DisplayWindow();

    using EventFunc = std::function<void(WindowEvent const&)>;

    void SetEventCallback(EventFunc const& func) { m_event_callback = func; }

    NODISCARD bool IsRunning() const;

    void StopRunning();

    void PollEvents();

    NODISCARD GLFWwindow* GetWindow() const { return m_window; }

    NODISCARD VkSurfaceKHR GetWindowSurface() const { return m_surface; }

private:

    GraphicsAPI const& m_api;
    GLFWwindow* m_window;
    EventFunc m_event_callback;
    VkSurfaceKHR m_surface;
};
