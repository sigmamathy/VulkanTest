#include "Graphics/Buffer.hpp"
#include "Graphics/Device.hpp"

#define THISFILE "Graphics/Buffer.cpp"

static uint32_t s_FindMemoryTypeIndex(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(device, &mp);

    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
    {
        if (filter & (1 << i) && (mp.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }

    ERRCHECK(false);
}

static std::pair<VkBuffer, VkDeviceMemory>
s_CreateBufferAndAllocateMemory(VkDevice device, VkPhysicalDevice pd, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
{
    std::pair<VkBuffer, VkDeviceMemory> result;

    VkBufferCreateInfo buffer_ci{};
    buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_ci.size = size;
    buffer_ci.usage = usage;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ERRCHECK(vkCreateBuffer(device, &buffer_ci, nullptr, &result.first) == VK_SUCCESS);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(device, result.first, &req);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = s_FindMemoryTypeIndex(pd, req.memoryTypeBits, props);

    ERRCHECK(vkAllocateMemory(device, &allocInfo, nullptr, &result.second) == VK_SUCCESS);

    vkBindBufferMemory(device, result.first, result.second, 0);

    return result;
}

static void
s_CopyViaStagingBuffer(GraphicsDevice const& device, VkDeviceSize size, void const* src, VkBuffer dst)
{
    auto [stage, stage_mem] = s_CreateBufferAndAllocateMemory(device.GetDevice(), device.GetPhysicalDevice(), size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(device.GetDevice(), stage_mem, 0, size, 0, &data);
    memcpy(data, src, size);
    vkUnmapMemory(device.GetDevice(), stage_mem);

    VkCommandBuffer copy = device.CreateTmpCmd([=](VkCommandBuffer cmd) -> void
    {
        VkBufferCopy buffer_copy{};
        buffer_copy.srcOffset = 0; // Optional
        buffer_copy.dstOffset = 0; // Optional
        buffer_copy.size = size;
        vkCmdCopyBuffer(cmd, stage, dst, 1, &buffer_copy);
    });

    VkSubmitInfo submit_i{};
    submit_i.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_i.commandBufferCount = 1;
    submit_i.pCommandBuffers = &copy;

    auto queue = device.GetGraphicsQueue().Queue;

    vkQueueSubmit(queue, 1, &submit_i, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    device.FreeTmpCmd(copy);

    vkDestroyBuffer(device.GetDevice(), stage, nullptr);
    vkFreeMemory(device.GetDevice(), stage_mem, nullptr);
}

VertexBuffer::VertexBuffer(GraphicsDevice const& device, VkDeviceSize size)
    : m_device(device), m_size(size)
{
    auto [fst, snd] = s_CreateBufferAndAllocateMemory(m_device.GetDevice(), m_device.GetPhysicalDevice(), size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_buffer = fst;
    m_memory = snd;
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(m_device.GetDevice(), m_buffer, nullptr);
    vkFreeMemory(m_device.GetDevice(), m_memory, nullptr);
}

void VertexBuffer::MapData(const void *src)
{
    s_CopyViaStagingBuffer(m_device, m_size, src, m_buffer);
}

IndexBuffer::IndexBuffer(GraphicsDevice const& device, VkDeviceSize size)
    : m_device(device), m_size(size)
{
    auto [fst, snd] = s_CreateBufferAndAllocateMemory(m_device.GetDevice(), m_device.GetPhysicalDevice(), size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_buffer = fst;
    m_memory = snd;
}

IndexBuffer::~IndexBuffer()
{
    vkDestroyBuffer(m_device.GetDevice(), m_buffer, nullptr);
    vkFreeMemory(m_device.GetDevice(), m_memory, nullptr);
}

void IndexBuffer::MapData(const unsigned *src)
{
    s_CopyViaStagingBuffer(m_device, m_size, src, m_buffer);
}

