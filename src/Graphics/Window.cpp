#include "Graphics/Window.hpp"
#include "Graphics/API.hpp"

#define THISFILE "Graphics/Window.cpp"

DisplayWindow::DisplayWindow(GraphicsAPI const& api, int width, int height, char const *title)
    : m_api(api)
{
    glfwWindowHint(GLFW_RESIZABLE, false);
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    ERRCHECK(m_window);

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
