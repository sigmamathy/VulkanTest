#pragma once

#include "Dependencies.hpp"

CLASS_DECLARE(GraphicsAPI);
CLASS_DECLARE(DisplayWindow);
CLASS_DECLARE(GraphicsPipeline);
CLASS_DECLARE(VertexBuffer);
CLASS_DECLARE(IndexBuffer);

struct CommandQueue
{
    VkQueue Queue;
    uint32_t FamilyIndex;
};

struct DrawCmdRecorder
{
    VkCommandBuffer Buffer;
    VkExtent2D Extent;

    void BindPipeline(GraphicsPipeline const& pipeline);
    void BindVertexBuffer(VertexBuffer const& buffer);
    void BindIndexBuffer(IndexBuffer const& buffer);

    void SetViewport(const VkViewport &viewport);
    void SetViewportDefault();
    void SetScissor(VkRect2D scissor);
    void SetScissorDefault();

    void Draw(uint32_t count, uint32_t instance);
    void DrawIndexed(uint32_t count, uint32_t instance);

    void EndRecord();
};

class GraphicsDevice
{
public:

    GraphicsDevice(GraphicsAPI const& api, DisplayWindow const& window);

    ~GraphicsDevice();

    void WaitIdle();

    void CreateDrawCmdBuffers(VkCommandBuffer* buffers, size_t count);

    DrawCmdRecorder BeginRecord(VkCommandBuffer buffer, size_t frame_index);

    void ResetRecord(VkCommandBuffer buffer);

    VkCommandBuffer CreateTmpCmd(std::function<void(VkCommandBuffer)> const& rec) const;

    void FreeTmpCmd(VkCommandBuffer buffer) const;

    NODISCARD VkPhysicalDevice GetPhysicalDevice() const { return m_physical_device; }

    NODISCARD VkDevice GetDevice() const { return m_device; }

    NODISCARD VkRenderPass GetRenderPass() const { return m_render_pass; }

    NODISCARD VkSwapchainKHR GetSwapchain() const { return m_swapchain; }

    NODISCARD CommandQueue GetGraphicsQueue() const { return m_graphics_queue; }

    NODISCARD CommandQueue GetPresentQueue() const { return m_present_queue; }

private:

    void InitDeviceAndQueue(GraphicsAPI const& api, DisplayWindow const& window);
    void InitSwapchain(DisplayWindow const& window);
    void InitCommandPool();
    void InitRenderPassAndFramebuffers();

    VkPhysicalDevice m_physical_device;
    VkDevice m_device;

    CommandQueue m_graphics_queue;
    CommandQueue m_present_queue;

    VkSwapchainKHR m_swapchain;
    VkFormat m_swapchain_image_format;
    VkExtent2D m_swapchain_extent;
    std::vector<VkImageView> m_swapchain_image_views;

    VkCommandPool m_draw_pool, m_tmp_pool;

    VkRenderPass m_render_pass;
    std::vector<VkFramebuffer> m_framebuffers;
};