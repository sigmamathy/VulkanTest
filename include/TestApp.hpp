#pragma once

#include "Dependencies.hpp"

struct CommandQueue
{
    VkQueue Queue;
    uint32_t FamilyIndex;
};

class TestApp
{
public:

    TestApp();
    ~TestApp();

    [[nodiscard]] bool IsRunning() const;
    void StopRunning();
    void PollEvents();

    void WaitIdle();

    [[nodiscard]] VkInstance GetInstance() const { return m_vkinst; }
    [[nodiscard]] GLFWwindow* GetWindow() const { return m_window; }
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_physical_device; }
    [[nodiscard]] VkDevice GetDevice() const { return m_device; }
    [[nodiscard]] CommandQueue GetGraphicsQueue() const { return m_graphics; }
    [[nodiscard]] CommandQueue GetPresentQueue() const { return m_present; }

    [[nodiscard]] VkSwapchainKHR GetSwapchain() const { return m_swapchain; }
    [[nodiscard]] VkFormat GetSwapchainImageFormat() const { return m_swapchain_image_format; }
    [[nodiscard]] VkExtent2D GetSwapchainExtent() const { return m_swapchain_extent; }
    [[nodiscard]] std::vector<VkImageView> const& GetSwapchainImageViews() const { return m_swapchain_image_views; }

private:

    void InitWindow();
    void InitVulkan();
    void InitWindowSurface();
    void InitDevice();
    void InitSwapchain();

    void FreeWindow();
    void FreeVulkan();
    void FreeWindowSurface();
    void FreeDevice();
    void FreeSwapchain();

    GLFWwindow* m_window;
    VkInstance m_vkinst;
    VkDebugUtilsMessengerEXT m_debug_messenger;
    VkSurfaceKHR m_surface;

    VkPhysicalDevice m_physical_device;
    VkDevice m_device;

    CommandQueue m_graphics;
    CommandQueue m_present;

    VkSwapchainKHR m_swapchain;
    VkFormat m_swapchain_image_format;
    VkExtent2D m_swapchain_extent;
    std::vector<VkImageView> m_swapchain_image_views;
};
