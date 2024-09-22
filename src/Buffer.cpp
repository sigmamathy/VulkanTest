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

VertexBuffer::VertexBuffer(TestApp const& app, VkDeviceSize size)
    : m_device(app.GetDevice()), m_size(size)
{
    VkBufferCreateInfo buffer_ci{};
    buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_ci.size = size;
    buffer_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CHECK(vkCreateBuffer(m_device, &buffer_ci, nullptr, &m_buffer) == VK_SUCCESS);

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(m_device, m_buffer, &req);

    VkMemoryAllocateInfo alloc_i{};
    alloc_i.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_i.allocationSize = req.size;
    alloc_i.memoryTypeIndex = s_FindMemoryTypeIndex(app.GetPhysicalDevice(), req.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CHECK(vkAllocateMemory(m_device, &alloc_i, nullptr, &m_memory) == VK_SUCCESS);

    vkBindBufferMemory(m_device, m_buffer, m_memory, 0);
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
