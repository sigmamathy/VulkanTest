#pragma once

#include "Dependencies.hpp"

CLASS_DECLARE(GraphicsAPI);

class DisplayWindow
{
public:

    DisplayWindow(GraphicsAPI const& api, int width, int height, char const* title);

    ~DisplayWindow();

    NODISCARD bool IsRunning() const;

    void StopRunning();

    void PollEvents();

    NODISCARD GLFWwindow* GetWindow() const { return m_window; }

    NODISCARD VkSurfaceKHR GetWindowSurface() const { return m_surface; }

private:

    GraphicsAPI const& m_api;
    GLFWwindow* m_window;
    VkSurfaceKHR m_surface;
};
