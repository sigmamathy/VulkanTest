#include "Graphics/Device.hpp"
#include "Graphics/API.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/Buffer.hpp"

#define THISFILE "Graphics/Device.cpp"

GraphicsDevice::GraphicsDevice(GraphicsAPI const &api, DisplayWindow const& window)
{
    InitDeviceAndQueue(api, window);
    InitSwapchain(window);
    InitCommandPool();
    InitRenderPassAndFramebuffers();
}

GraphicsDevice::~GraphicsDevice()
{
    for (auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);

    vkDestroyCommandPool(m_device, m_draw_pool, nullptr);
    vkDestroyCommandPool(m_device, m_tmp_pool, nullptr);

    for (auto view : m_swapchain_image_views)
        vkDestroyImageView(m_device, view, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vkDestroyDevice(m_device, nullptr);
}

void GraphicsDevice::WaitIdle()
{
    vkDeviceWaitIdle(m_device);
}

size_t GraphicsDevice::CreateDrawCmdBuffers(size_t count)
{
    size_t index = m_cmd_buffers.size();
    m_cmd_buffers.resize(index + count);

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_draw_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = static_cast<uint32_t>(count);

    ERRCHECK(vkAllocateCommandBuffers(m_device, &info, m_cmd_buffers.data() + index) == VK_SUCCESS);
    return index;
}

DrawCmdRecorder GraphicsDevice::BeginRecord(size_t index, size_t frame_index)
{
    VkCommandBuffer buffer = m_cmd_buffers[index];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    ERRCHECK(vkBeginCommandBuffer(buffer, &beginInfo) == VK_SUCCESS);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_render_pass;
    renderPassInfo.framebuffer = m_framebuffers[frame_index];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchain_extent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    return DrawCmdRecorder{buffer, m_swapchain_extent};
}

void GraphicsDevice::ResetRecord(size_t index)
{
    vkResetCommandBuffer(m_cmd_buffers[index], 0);
}

void DrawCmdRecorder::BindPipeline(GraphicsPipeline const &pipeline)
{
    vkCmdBindPipeline(Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());
}

void DrawCmdRecorder::BindVertexBuffer(VertexBuffer const &buffer)
{
    VkBuffer b = buffer.GetBuffer();
    VkDeviceSize off = 0;
    vkCmdBindVertexBuffers(Buffer, 0, 1, &b, &off);
}

void DrawCmdRecorder::BindIndexBuffer(IndexBuffer const &buffer)
{
    vkCmdBindIndexBuffer(Buffer, buffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void DrawCmdRecorder::SetViewport(VkViewport const& viewport)
{
    vkCmdSetViewport(Buffer, 0, 1, &viewport);
}

void DrawCmdRecorder::SetViewportDefault()
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

void DrawCmdRecorder::SetScissor(VkRect2D scissor)
{
    vkCmdSetScissor(Buffer, 0, 1, &scissor);
}

void DrawCmdRecorder::SetScissorDefault()
{
    VkRect2D scissor = {
        {0, 0},
        Extent
    };

    SetScissor(scissor);
}

void DrawCmdRecorder::Draw(uint32_t count, uint32_t instance)
{
    vkCmdDraw(Buffer, count, instance, 0, 0);
}

void DrawCmdRecorder::DrawIndexed(uint32_t count, uint32_t instance)
{
    vkCmdDrawIndexed(Buffer, count, instance, 0, 0, 0);
}

void DrawCmdRecorder::EndRecord()
{
    vkCmdEndRenderPass(Buffer);
    ERRCHECK(vkEndCommandBuffer(Buffer) == VK_SUCCESS);
}

void GraphicsDevice::InitDeviceAndQueue(GraphicsAPI const &api, DisplayWindow const &window)
{
    VkInstance instance = api.GetInstance();

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    ERRCHECK(device_count);
    device_count = 1;
    vkEnumeratePhysicalDevices(instance, &device_count, &m_physical_device);

    constexpr std::array device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> families(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &family_count, families.data());

    bool graphics_ok = false;
    bool present_ok = false;

    for (int i = 0; i < families.size() && !(graphics_ok && present_ok); i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphics_queue.FamilyIndex = i;
            graphics_ok = true;
        }

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, window.GetWindowSurface(), &present_support);
        if (present_support) {
            m_present_queue.FamilyIndex = i;
            present_ok = true;
        }
    }

    ERRCHECK(graphics_ok && present_ok);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    float priority = 1.0f;

    for (uint32_t index : std::unordered_set{m_graphics_queue.FamilyIndex, m_present_queue.FamilyIndex})
    {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = index;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
        queue_create_infos.push_back(info);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_ci{};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.pQueueCreateInfos = queue_create_infos.data();
    device_ci.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_ci.pEnabledFeatures = &device_features;
    device_ci.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_ci.ppEnabledExtensionNames = device_extensions.data();

#ifndef NDEBUG

    auto validation_layer = GRAPHICS_VALIDATION_LAYER;
    device_ci.enabledLayerCount = 1;
    device_ci.ppEnabledLayerNames = &validation_layer;

#else

    device_ci.enabledLayerCount = 0;

#endif

    ERRCHECK(vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device) == VK_SUCCESS);

    vkGetDeviceQueue(m_device, m_graphics_queue.FamilyIndex, 0, &m_graphics_queue.Queue);
    vkGetDeviceQueue(m_device, m_present_queue.FamilyIndex, 0, &m_present_queue.Queue);
}

static VkSurfaceFormatKHR s_ChooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (auto const& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return formats[0];
}

static VkPresentModeKHR s_ChoosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::vector<VkPresentModeKHR> modes;
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    modes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (auto const& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D s_ChooseSwapExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR const& caps)
{
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return caps.currentExtent;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actual.width = std::clamp(actual.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    actual.height = std::clamp(actual.height, caps.minImageExtent.height, caps.maxImageExtent.height);

    return actual;
}

void GraphicsDevice::InitSwapchain(DisplayWindow const& window)
{
    VkSurfaceKHR surface = window.GetWindowSurface();

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface, &caps);

    auto [format, color_space]      = s_ChooseSurfaceFormat(m_physical_device, surface);
    VkPresentModeKHR present_mode   = s_ChoosePresentMode(m_physical_device, surface);
    VkExtent2D extent               = s_ChooseSwapExtent(window.GetWindow(), caps);

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    VkSwapchainCreateInfoKHR swapchain_ci{};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.surface = surface;
    swapchain_ci.minImageCount = image_count;
    swapchain_ci.imageFormat = format;
    swapchain_ci.imageColorSpace = color_space;
    swapchain_ci.imageExtent = extent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = { m_graphics_queue.FamilyIndex, m_present_queue.FamilyIndex };

    if (m_graphics_queue.FamilyIndex != m_present_queue.FamilyIndex)
    {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_ci.queueFamilyIndexCount = 2;
        swapchain_ci.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_ci.queueFamilyIndexCount = 0;
        swapchain_ci.pQueueFamilyIndices = nullptr;
    }

    swapchain_ci.preTransform = caps.currentTransform;
    swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.presentMode = present_mode;
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;

    ERRCHECK(vkCreateSwapchainKHR(m_device, &swapchain_ci, nullptr, &m_swapchain) == VK_SUCCESS);

    m_swapchain_image_format = format;
    m_swapchain_extent = extent;

    std::vector<VkImage> images;
    images.resize(image_count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, images.data());
    m_swapchain_image_views.resize(image_count);

    for (uint32_t i = 0; i < image_count; i++)
    {
        VkImageViewCreateInfo view_ci{};
        view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_ci.image = images[i];
        view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_ci.format = format;
        view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_ci.subresourceRange.baseMipLevel = 0;
        view_ci.subresourceRange.levelCount = 1;
        view_ci.subresourceRange.baseArrayLayer = 0;
        view_ci.subresourceRange.layerCount = 1;

        ERRCHECK(vkCreateImageView(m_device, &view_ci, nullptr, &m_swapchain_image_views[i]) == VK_SUCCESS);
    }
}

void GraphicsDevice::InitCommandPool()
{
    m_cmd_buffers.reserve(8);

    VkCommandPoolCreateInfo pool_ci{};
    pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_ci.queueFamilyIndex = m_graphics_queue.FamilyIndex;

    ERRCHECK(
        vkCreateCommandPool(m_device, &pool_ci, nullptr, &m_draw_pool) == VK_SUCCESS
        && vkCreateCommandPool(m_device, &pool_ci, nullptr, &m_tmp_pool) == VK_SUCCESS);
}

void GraphicsDevice::InitRenderPassAndFramebuffers()
{
    VkAttachmentDescription description{};
    description.format = m_swapchain_image_format;
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference reference{};
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &reference;

    VkRenderPassCreateInfo render_pass_ci{};
    render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &description;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    ERRCHECK(vkCreateRenderPass(m_device, &render_pass_ci, nullptr, &m_render_pass) == VK_SUCCESS);

    m_framebuffers.resize(m_swapchain_image_views.size());

    for (int i = 0; i < m_framebuffers.size(); i++)
    {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_swapchain_image_views[i];
        framebufferInfo.width = m_swapchain_extent.width;
        framebufferInfo.height = m_swapchain_extent.height;
        framebufferInfo.layers = 1;

        ERRCHECK(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) == VK_SUCCESS);
    }
}
