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

    CHECK(vkCreateBuffer(device, &buffer_ci, nullptr, &result.first) != VK_SUCCESS);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(device, result.first, &req);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = s_FindMemoryTypeIndex(pd, req.memoryTypeBits, props);

    CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &result.second) != VK_SUCCESS);

    vkBindBufferMemory(device, result.first, result.second, 0);

    return result;
}

VertexBuffer::VertexBuffer(TestApp const& app, VkDeviceSize size)
    : m_device(app.GetDevice()), m_size(size)
{
    auto [fst, snd] = s_CreateBufferAndAllocateMemory(m_device, app.GetPhysicalDevice(), size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_buffer = fst;
    m_memory = snd;
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
}

void VertexBuffer::MapData(const void *src)
{
    void* data;
    vkMapMemory(m_device, m_memory, 0, m_size, 0, &data);
    memcpy(data, src, m_size);
    vkUnmapMemory(m_device, m_memory);
}
