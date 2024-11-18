#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <iostream>

class Vulkan_Instance;

class Vulkan_Debugger
{
public:
    Vulkan_Debugger(const std::unique_ptr<Vulkan_Instance>& instance);

private:
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger;

    vk::DebugUtilsMessengerCreateInfoEXT GetCreateInfo();

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanMessengerUtilCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};