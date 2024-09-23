#pragma once

#include "Dependencies.hpp"

CLASS_DECLARE(GraphicsDevice);

class DrawPresentSynchronizer
{
public:

    explicit DrawPresentSynchronizer(GraphicsDevice const& device);

    ~DrawPresentSynchronizer();

    uint32_t NextFrame();

    void SubmitDraw(VkCommandBuffer buffer);

    void PresentOnScreen(uint32_t frame_index);

private:

    GraphicsDevice const& m_device;

    VkSemaphore m_image_available;
    VkSemaphore m_render_finished;
    VkFence m_in_flight;
};