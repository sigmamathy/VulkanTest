#pragma once

#include "Dependencies.hpp"

class GraphicsAPI
{
public:

    GraphicsAPI();

    ~GraphicsAPI();

    NODISCARD VkInstance GetInstance() const { return m_instance; }

private:

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;

};