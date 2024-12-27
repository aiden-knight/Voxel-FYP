#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <iostream>

#include "Vulkan_FWD.h"

class Vulkan_Debugger
{
public:
    Vulkan_Debugger(InstancePtr instance);

private:
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger;

    vk::DebugUtilsMessengerCreateInfoEXT GetCreateInfo();

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanMessengerUtilCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};