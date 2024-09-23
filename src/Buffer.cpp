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

    CHECK(false);
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

    CHECK(vkCreateBuffer(device, &buffer_ci, nullptr, &result.first) == VK_SUCCESS);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(device, result.first, &req);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = s_FindMemoryTypeIndex(pd, req.memoryTypeBits, props);

    CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &result.second) == VK_SUCCESS);

    vkBindBufferMemory(device, result.first, result.second, 0);

    return result;
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
    auto [stage, stage_mem] = s_CreateBufferAndAllocateMemory(m_device, m_physical_device, m_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(m_device, stage_mem, 0, m_size, 0, &data);
    memcpy(data, src, m_size);
    vkUnmapMemory(m_device, stage_mem);

    VkCommandBufferAllocateInfo cmd_i{};
    cmd_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_i.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_i.commandPool = pool;
    cmd_i.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(m_device, &cmd_i, &cmd);

    VkCommandBufferBeginInfo begin_i{};
    begin_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_i.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &begin_i);

    VkBufferCopy buffer_copy{};
    buffer_copy.srcOffset = 0; // Optional
    buffer_copy.dstOffset = 0; // Optional
    buffer_copy.size = m_size;
    vkCmdCopyBuffer(cmd, stage, m_buffer, 1, &buffer_copy);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_i{};
    submit_i.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_i.commandBufferCount = 1;
    submit_i.pCommandBuffers = &cmd;

    vkQueueSubmit(m_graphics_queue, 1, &submit_i, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_device, pool, 1, &cmd);

    vkDestroyBuffer(m_device, stage, nullptr);
    vkFreeMemory(m_device, stage_mem, nullptr);
}
