#pragma once

#include "Dependencies.hpp"

struct DrawRecorder
{
    VkCommandBuffer Buffer;
    VkExtent2D Extent;

    void BindPipeline(class GraphicsPipeline const& pipeline);
    void BindVertexBuffer(class VertexBuffer const& buffer);

    void SetViewport(const VkViewport &viewport);
    void SetViewportDefault();
    void SetScissor(VkRect2D scissor);
    void SetScissorDefault();

    void Draw(uint32_t count, uint32_t instance);

    void EndRecord();
};

class DrawCommandPool
{
public:

    explicit DrawCommandPool(class TestApp const& app);
    ~DrawCommandPool();

    size_t CreateBuffers(size_t count);

    [[nodiscard]] VkCommandBuffer GetCommandBuffer(size_t index) const { return m_buffers[index]; }

    DrawRecorder BeginRecord(size_t index, class Framebuffers const& framebuffers, size_t frame_index);

    void ResetRecord(size_t index);

    [[nodiscard]] VkCommandPool GetPool() const { return m_pool; }

private:

    VkDevice m_device;
    VkCommandPool m_pool;
    std::vector<VkCommandBuffer> m_buffers;

};

class DrawPresentSynchronizer
{
public:

    explicit DrawPresentSynchronizer(TestApp const& app);

    ~DrawPresentSynchronizer();

    uint32_t NextFrame();

    void SubmitDraw(VkCommandBuffer buffer);

    void PresentOnScreen(uint32_t frame_index);

private:

    VkDevice m_device;
    VkQueue m_graphics_queue, m_present_queue;
    VkSwapchainKHR m_swapchain;

    VkSemaphore m_image_available;
    VkSemaphore m_render_finished;
    VkFence m_in_flight;
};