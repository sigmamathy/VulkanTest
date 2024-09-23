#include "Drawing.hpp"
#include "TestApp.hpp"
#include "Graphics.hpp"
#include "Buffer.hpp"

#define THISFILE "Drawing.cpp"

DrawCommandPool::DrawCommandPool(TestApp const& app)
    : m_device(app.GetDevice()), m_pool{}
{
    m_buffers.reserve(8);

    VkCommandPoolCreateInfo pool_ci{};
    pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_ci.queueFamilyIndex = app.GetGraphicsQueue().FamilyIndex;

    ERRCHECK(vkCreateCommandPool(m_device, &pool_ci, nullptr, &m_pool) == VK_SUCCESS);
}

DrawCommandPool::~DrawCommandPool()
{
    vkDestroyCommandPool(m_device, m_pool, nullptr);
}

size_t DrawCommandPool::CreateBuffers(size_t count)
{
    size_t index = m_buffers.size();
    m_buffers.resize(index + count);

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(count);

    ERRCHECK(vkAllocateCommandBuffers(m_device, &info, m_buffers.data() + index) == VK_SUCCESS);
    return index;
}

DrawRecorder DrawCommandPool::BeginRecord(size_t index, Framebuffers const& framebuffers, size_t frame_index)
{
    VkCommandBuffer buffer = m_buffers[index];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    ERRCHECK(vkBeginCommandBuffer(buffer, &beginInfo) == VK_SUCCESS);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = framebuffers.GetRenderPass();
    renderPassInfo.framebuffer = framebuffers.GetFramebuffer(frame_index);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = framebuffers.GetExtent();

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    return DrawRecorder{buffer, framebuffers.GetExtent()};
}

void DrawCommandPool::ResetRecord(size_t index)
{
    vkResetCommandBuffer(m_buffers[index], 0);
}

void DrawRecorder::BindPipeline(GraphicsPipeline const &pipeline)
{
    vkCmdBindPipeline(Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());
}

void DrawRecorder::BindVertexBuffer(VertexBuffer const &buffer)
{
    VkBuffer b = buffer.GetBuffer();
    VkDeviceSize off = 0;
    vkCmdBindVertexBuffers(Buffer, 0, 1, &b, &off);
}

void DrawRecorder::BindIndexBuffer(IndexBuffer const &buffer)
{
    vkCmdBindIndexBuffer(Buffer, buffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void DrawRecorder::SetViewport(VkViewport const& viewport)
{
    vkCmdSetViewport(Buffer, 0, 1, &viewport);
}

void DrawRecorder::SetViewportDefault()
{
    VkViewport viewport = {
        0.f,
        0.f,
        static_cast<float>(Extent.width),
        static_cast<float>(Extent.height),
        0.f,
        1.f
    };

    SetViewport(viewport);
}

void DrawRecorder::SetScissor(VkRect2D scissor)
{
    vkCmdSetScissor(Buffer, 0, 1, &scissor);
}

void DrawRecorder::SetScissorDefault()
{
    VkRect2D scissor = {
        {0, 0},
        Extent
    };

    SetScissor(scissor);
}

void DrawRecorder::Draw(uint32_t count, uint32_t instance)
{
    vkCmdDraw(Buffer, count, instance, 0, 0);
}

void DrawRecorder::DrawIndexed(uint32_t count, uint32_t instance)
{
    vkCmdDrawIndexed(Buffer, count, instance, 0, 0, 0);
}

void DrawRecorder::EndRecord()
{
    vkCmdEndRenderPass(Buffer);
    ERRCHECK(vkEndCommandBuffer(Buffer) == VK_SUCCESS);
}

DrawPresentSynchronizer::DrawPresentSynchronizer(TestApp const& app)
    : m_device(app.GetDevice()), m_graphics_queue(app.GetGraphicsQueue().Queue), m_present_queue(app.GetPresentQueue().Queue),
    m_swapchain(app.GetSwapchain()), m_image_available{}, m_render_finished{}, m_in_flight{}
{
    VkSemaphoreCreateInfo semaphore_ci{};
    semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_ci{};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    ERRCHECK(
        vkCreateSemaphore(m_device, &semaphore_ci, nullptr, &m_image_available) == VK_SUCCESS
        && vkCreateSemaphore(m_device, &semaphore_ci, nullptr, &m_render_finished) == VK_SUCCESS
        && vkCreateFence(m_device, &fence_ci, nullptr, &m_in_flight) == VK_SUCCESS);
}

DrawPresentSynchronizer::~DrawPresentSynchronizer()
{
    vkDestroySemaphore(m_device, m_image_available, nullptr);
    vkDestroySemaphore(m_device, m_render_finished, nullptr);
    vkDestroyFence(m_device, m_in_flight, nullptr);
}

uint32_t DrawPresentSynchronizer::NextFrame()
{
    vkWaitForFences(m_device, 1, &m_in_flight, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_in_flight);

    uint32_t index;
    vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_available, VK_NULL_HANDLE, &index);
    return index;
}

void DrawPresentSynchronizer::SubmitDraw(VkCommandBuffer buffer)
{
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_image_available;
    submit.pWaitDstStageMask = wait_stages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &buffer;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &m_render_finished;

    ERRCHECK(vkQueueSubmit(m_graphics_queue, 1, &submit, m_in_flight) == VK_SUCCESS);
}

void DrawPresentSynchronizer::PresentOnScreen(uint32_t frame_index)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_render_finished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &frame_index;

    ERRCHECK(vkQueuePresentKHR(m_present_queue, &presentInfo) == VK_SUCCESS);
}

