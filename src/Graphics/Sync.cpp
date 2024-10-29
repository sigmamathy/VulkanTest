#include "Graphics/Sync.hpp"

#include "Graphics/Device.hpp"

#define THISFILE "Graphics/Sync.cpp"

DrawPresentSynchronizer::DrawPresentSynchronizer(GraphicsDevice const& device)
    : m_device(device), m_image_available{}, m_render_finished{}, m_in_flight{}
{
    VkDevice dev = device.GetDevice();

    VkSemaphoreCreateInfo semaphore_ci{};
    semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_ci{};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    ERRCHECK(
        vkCreateSemaphore(dev, &semaphore_ci, nullptr, &m_image_available) == VK_SUCCESS
        && vkCreateSemaphore(dev, &semaphore_ci, nullptr, &m_render_finished) == VK_SUCCESS
        && vkCreateFence(dev, &fence_ci, nullptr, &m_in_flight) == VK_SUCCESS);
}

DrawPresentSynchronizer::~DrawPresentSynchronizer()
{
    VkDevice dev = m_device.GetDevice();
    vkDestroySemaphore(dev, m_image_available, nullptr);
    vkDestroySemaphore(dev, m_render_finished, nullptr);
    vkDestroyFence(dev, m_in_flight, nullptr);
}

uint32_t DrawPresentSynchronizer::NextFrame()
{
    VkDevice dev = m_device.GetDevice();
    vkWaitForFences(dev, 1, &m_in_flight, VK_TRUE, UINT64_MAX);
    vkResetFences(dev, 1, &m_in_flight);

    uint32_t index;
    vkAcquireNextImageKHR(dev, m_device.GetSwapchain(), UINT64_MAX, m_image_available, VK_NULL_HANDLE, &index);
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

    ERRCHECK(vkQueueSubmit(m_device.GetGraphicsQueue().Queue, 1, &submit, m_in_flight) == VK_SUCCESS);
}

void DrawPresentSynchronizer::PresentOnScreen(uint32_t frame_index)
{
    VkSwapchainKHR swapchains[] = { m_device.GetSwapchain() };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_render_finished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &frame_index;

    VkResult r = vkQueuePresentKHR(m_device.GetPresentQueue().Queue, &presentInfo);
    ERRCHECK(r == VK_SUCCESS || r == VK_SUBOPTIMAL_KHR);
}