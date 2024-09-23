#pragma once

#include "Dependencies.hpp"

class VertexBuffer
{
public:

    VertexBuffer(class TestApp const& app, VkDeviceSize size);
    ~VertexBuffer();

    void MapData(VkCommandPool pool, const void* src);

    [[nodiscard]] VkBuffer GetBuffer() const { return m_buffer; }

private:

    VkDevice m_device;
    VkPhysicalDevice m_physical_device;
    VkQueue m_graphics_queue;

    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};

class IndexBuffer
{
public:

    IndexBuffer(TestApp const& app, VkDeviceSize size);
    ~IndexBuffer();

    void MapData(VkCommandPool pool, const unsigned* src);

    [[nodiscard]] VkBuffer GetBuffer() const { return m_buffer; }


private:

    VkDevice m_device;
    VkPhysicalDevice m_physical_device;
    VkQueue m_graphics_queue;

    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};