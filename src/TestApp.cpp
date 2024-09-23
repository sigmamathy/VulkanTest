#include "TestApp.hpp"

#define THISFILE "TestApp.cpp"
#define VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

TestApp::TestApp()
    : m_window{}, m_vkinst{}, m_debug_messenger{}, m_surface{},
    m_physical_device{}, m_device{}, m_graphics{}, m_present{},
    m_swapchain{}, m_swapchain_image_format{}, m_swapchain_extent{}
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    InitWindow();
    InitVulkan();
    InitWindowSurface();
    InitDevice();
    InitSwapchain();
}

TestApp::~TestApp()
{
    FreeSwapchain();
    FreeDevice();
    FreeWindowSurface();
    FreeVulkan();
    FreeWindow();
    glfwTerminate();
}

void TestApp::InitWindow()
{
    glfwWindowHint(GLFW_RESIZABLE, false);
    m_window = glfwCreateWindow(1600, 900, "Hello World", nullptr, nullptr);
    ERRCHECK(m_window);
}

void TestApp::FreeWindow()
{
    glfwDestroyWindow(m_window);
}

#ifndef NDEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL
s_DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                       VkDebugUtilsMessageTypeFlagsEXT type,
                       const VkDebugUtilsMessengerCallbackDataEXT* data,
                       void* user_data)
{
    auto& os = severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ? std::cerr : std::cout;
    os << "[Vulkan:" << severity << "] " << data->pMessage << '\n';
    return VK_FALSE;
}

#endif

void TestApp::InitVulkan()
{
    VkApplicationInfo app_i{};
    app_i.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_i.pApplicationName = "Vulkan Example";
    app_i.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_i.pEngineName = "No Engine";
    app_i.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_i.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo inst_ci{};
    inst_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_ci.pApplicationInfo = &app_i;

    uint32_t glfw_ext_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

#ifndef NDEBUG

    auto constexpr validation_layer = VALIDATION_LAYER;

    // check if system support VK_LAYER_KHRONOS_validation layer
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> av_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, av_layers.data());
    ERRCHECK(std::find_if(av_layers.begin(), av_layers.end(),
        [](VkLayerProperties const& prop) -> bool {
            return std::strcmp(prop.layerName, validation_layer) == 0;
        }) != av_layers.end());

    // setup validation layer
    inst_ci.enabledLayerCount = 1;
    inst_ci.ppEnabledLayerNames = &validation_layer;

    // setup extensions (glfw + debug)
    std::vector extensions(glfw_extensions, glfw_extensions + glfw_ext_count);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    inst_ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    inst_ci.ppEnabledExtensionNames = extensions.data();

    // setup debug messenger parameters
    VkDebugUtilsMessengerCreateInfoEXT debug_ci{};
    debug_ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_ci.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_ci.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_ci.pfnUserCallback = s_DebugMessageCallback;
    debug_ci.pUserData = nullptr;
    inst_ci.pNext = &debug_ci;

#else

    // setup extensions (glfw)
    inst_ci.enabledLayerCount = 0;
    inst_ci.enabledExtensionCount = glfw_ext_count;
    inst_ci.ppEnabledExtensionNames = glfw_extensions;

#endif

    ERRCHECK(vkCreateInstance(&inst_ci, nullptr, &m_vkinst) == VK_SUCCESS);

#ifndef NDEBUG

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_vkinst, "vkCreateDebugUtilsMessengerEXT"));
    ERRCHECK(func);
    func(m_vkinst, &debug_ci, nullptr, &m_debug_messenger);

#endif
}

void TestApp::FreeVulkan()
{
#ifndef NDEBUG

    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_vkinst, "vkDestroyDebugUtilsMessengerEXT"));
    ERRCHECK(func);
    func(m_vkinst, m_debug_messenger, nullptr);

#endif

    vkDestroyInstance(m_vkinst, nullptr);
}

void TestApp::InitWindowSurface()
{
    ERRCHECK(glfwCreateWindowSurface(m_vkinst, m_window, nullptr, &m_surface) == VK_SUCCESS);
}

void TestApp::FreeWindowSurface()
{
    vkDestroySurfaceKHR(m_vkinst, m_surface, nullptr);
}

void TestApp::InitDevice()
{
    // only query the first gpu available
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_vkinst, &device_count, nullptr);
    ERRCHECK(device_count);
    device_count = 1;
    vkEnumeratePhysicalDevices(m_vkinst, &device_count, &m_physical_device);

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
            m_graphics.FamilyIndex = i;
            graphics_ok = true;
        }

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, m_surface, &present_support);
        if (present_support) {
            m_present.FamilyIndex = i;
            present_ok = true;
        }
    }

    ERRCHECK(graphics_ok && present_ok);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    float priority = 1.0f;

    for (uint32_t index : std::unordered_set{m_graphics.FamilyIndex, m_present.FamilyIndex})
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

    auto validation_layer = VALIDATION_LAYER;
    device_ci.enabledLayerCount = 1;
    device_ci.ppEnabledLayerNames = &validation_layer;

#else

    device_ci.enabledLayerCount = 0;

#endif

    ERRCHECK(vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device) == VK_SUCCESS);

    vkGetDeviceQueue(m_device, m_graphics.FamilyIndex, 0, &m_graphics.Queue);
    vkGetDeviceQueue(m_device, m_present.FamilyIndex, 0, &m_present.Queue);
}

void TestApp::FreeDevice()
{
    vkDestroyDevice(m_device, nullptr);
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

static VkPresentModeKHR s_ChoosePresentModes(VkPhysicalDevice device, VkSurfaceKHR surface)
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

void TestApp::InitSwapchain()
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &caps);

    auto [format, color_space]      = s_ChooseSurfaceFormat(m_physical_device, m_surface);
    VkPresentModeKHR present_mode   = s_ChoosePresentModes(m_physical_device, m_surface);
    VkExtent2D extent               = s_ChooseSwapExtent(m_window, caps);

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    VkSwapchainCreateInfoKHR swapchain_ci{};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.surface = m_surface;
    swapchain_ci.minImageCount = image_count;
    swapchain_ci.imageFormat = format;
    swapchain_ci.imageColorSpace = color_space;
    swapchain_ci.imageExtent = extent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = { m_graphics.FamilyIndex, m_present.FamilyIndex };

    if (m_graphics.FamilyIndex != m_present.FamilyIndex)
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

void TestApp::FreeSwapchain()
{
    for (auto view : m_swapchain_image_views)
        vkDestroyImageView(m_device, view, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}


bool TestApp::IsRunning() const
{
    return !glfwWindowShouldClose(m_window);
}

void TestApp::StopRunning()
{
    glfwSetWindowShouldClose(m_window, 1);
}

void TestApp::PollEvents()
{
    glfwPollEvents();
}

void TestApp::WaitIdle()
{
    vkDeviceWaitIdle(m_device);
}
