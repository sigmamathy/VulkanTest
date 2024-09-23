#include "Buffer.hpp"
#include "TestApp.hpp"

#define THISFILE "Buffer.cpp"

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
s_CopyViaStagingBuffer(VkDevice device, VkPhysicalDevice pd, VkDeviceSize size, VkCommandPool pool, VkQueue queue,
    void const* src, VkBuffer dst)
{
    auto [stage, stage_mem] = s_CreateBufferAndAllocateMemory(device, pd, size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(device, stage_mem, 0, size, 0, &data);
    memcpy(data, src, size);
    vkUnmapMemory(device, stage_mem);

    VkCommandBufferAllocateInfo cmd_i{};
    cmd_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_i.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_i.commandPool = pool;
    cmd_i.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &cmd_i, &cmd);

    VkCommandBufferBeginInfo begin_i{};
    begin_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_i.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &begin_i);

    VkBufferCopy buffer_copy{};
    buffer_copy.srcOffset = 0; // Optional
    buffer_copy.dstOffset = 0; // Optional
    buffer_copy.size = size;
    vkCmdCopyBuffer(cmd, stage, dst, 1, &buffer_copy);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_i{};
    submit_i.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_i.commandBufferCount = 1;
    submit_i.pCommandBuffers = &cmd;

    vkQueueSubmit(queue, 1, &submit_i, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, pool, 1, &cmd);

    vkDestroyBuffer(device, stage, nullptr);
    vkFreeMemory(device, stage_mem, nullptr);
}

VertexBuffer::VertexBuffer(TestApp const& app, VkDeviceSize size)
    : m_device(app.GetDevice()), m_physical_device(app.GetPhysicalDevice()),
    m_graphics_queue(app.GetGraphicsQueue().Queue), m_size(size)
{
    auto [fst, snd] = s_CreateBufferAndAllocateMemory(m_device, m_physical_device, size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_buffer = fst;
    m_memory = snd;
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

void VertexBuffer::MapData(VkCommandPool pool, const void *src)
{
    s_CopyViaStagingBuffer(m_device, m_physical_device, m_size, pool, m_graphics_queue, src, m_buffer);
}

IndexBuffer::IndexBuffer(TestApp const &app, VkDeviceSize size)
    : m_device(app.GetDevice()), m_physical_device(app.GetPhysicalDevice()),
    m_graphics_queue(app.GetGraphicsQueue().Queue), m_size(size)
{
    auto [fst, snd] = s_CreateBufferAndAllocateMemory(m_device, m_physical_device, size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_buffer = fst;
    m_memory = snd;
}

IndexBuffer::~IndexBuffer()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

void IndexBuffer::MapData(VkCommandPool pool, const unsigned *src)
{
    s_CopyViaStagingBuffer(m_device, m_physical_device, m_size, pool, m_graphics_queue, src, m_buffer);
}

