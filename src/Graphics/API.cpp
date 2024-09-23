#include "Graphics/API.hpp"

#define THISFILE "Graphics/API.cpp"

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

GraphicsAPI::GraphicsAPI(): m_instance(nullptr)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

#ifndef NDEBUG

    auto constexpr validation_layer = GRAPHICS_VALIDATION_LAYER;

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

    ERRCHECK(vkCreateInstance(&inst_ci, nullptr, &m_instance) == VK_SUCCESS);

#ifndef NDEBUG

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));
    ERRCHECK(func);
    func(m_instance, &debug_ci, nullptr, &m_debug_messenger);

#endif
}

GraphicsAPI::~GraphicsAPI()
{
#ifndef NDEBUG

    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
    //ERRCHECK(func);
    func(m_instance, m_debug_messenger, nullptr);

#endif

    vkDestroyInstance(m_instance, nullptr);
}
