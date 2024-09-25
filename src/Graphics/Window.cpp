#include "Graphics/Window.hpp"
#include "Graphics/API.hpp"

#define THISFILE "Graphics/Window.cpp"
#define GETEVFUNC(win) *static_cast<DisplayWindow::EventFunc*>(glfwGetWindowUserPointer(win))

static void s_CreateWindowCallback(GLFWwindow* window)
{
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) -> void {
        if (action == GLFW_REPEAT) return;
        auto& func = GETEVFUNC(win);
        WindowEvent ev;
        ev.Type = WindowEvent::KEY_EVENT;
        ev.KE.Code = key, ev.KE.Down = action;
        func(ev);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) -> void {
        auto& func = GETEVFUNC(win);
        WindowEvent ev;
        ev.Type = WindowEvent::MOUSE_EVENT;
        ev.ME.Button = button, ev.ME.Down = action;
        func(ev);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) -> void {
        auto& func = GETEVFUNC(win);
        WindowEvent ev;
        ev.Type = WindowEvent::CURSOR_EVENT;
        ev.CE.Pos = {xpos, ypos};
        func(ev);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoff, double yoff) -> void {
        auto& func = GETEVFUNC(win);
        WindowEvent ev;
        ev.Type = WindowEvent::SCROLL_EVENT;
        ev.SE.Offset = {xoff, yoff};
        func(ev);
    });
}

DisplayWindow::DisplayWindow(GraphicsAPI const& api, int width, int height, char const *title)
    : m_api(api), m_surface(nullptr)
{
    glfwWindowHint(GLFW_RESIZABLE, false);
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    ERRCHECK(m_window);
    m_event_callback = [](auto){};
    glfwSetWindowUserPointer(m_window, &m_event_callback);
    s_CreateWindowCallback(m_window);

    ERRCHECK(glfwCreateWindowSurface(api.GetInstance(), m_window, nullptr, &m_surface) == VK_SUCCESS);
}

DisplayWindow::~DisplayWindow()
{
    vkDestroySurfaceKHR(m_api.GetInstance(), m_surface, nullptr);
    glfwDestroyWindow(m_window);
}

bool DisplayWindow::IsRunning() const
{
    return !glfwWindowShouldClose(m_window);
}

void DisplayWindow::StopRunning()
{
    glfwSetWindowShouldClose(m_window, 1);
}

void DisplayWindow::PollEvents()
{
    glfwPollEvents();
}
