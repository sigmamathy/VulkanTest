#pragma once

#include "Dependencies.hpp"

CLASS_DECLARE(GraphicsDevice);

class VertexBuffer
{
public:

    VertexBuffer(GraphicsDevice const& device, VkDeviceSize size);

    ~VertexBuffer();

    void MapData(const void* src);

    NODISCARD VkBuffer GetBuffer() const { return m_buffer; }

    NODISCARD VkDeviceSize GetSize() const { return m_size; }

private:

    GraphicsDevice const& m_device;

    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};

class IndexBuffer
{
public:

    IndexBuffer(GraphicsDevice const& device, VkDeviceSize size);

    ~IndexBuffer();

    void MapData(const unsigned* src);

    NODISCARD VkBuffer GetBuffer() const { return m_buffer; }

    NODISCARD VkDeviceSize GetSize() const { return m_size; }

private:

    GraphicsDevice const& m_device;

    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};