#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <vector>

class Vulkan_Wrapper;

class Vulkan_Instance
{
public:
	Vulkan_Instance(const Vulkan_Wrapper* owner);

	vk::raii::DebugUtilsMessengerEXT CreateDebugUtilMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT createInfo) const;
private:
	vk::raii::Instance m_instance;

	vk::InstanceCreateInfo GetCreateInfo(const Vulkan_Wrapper* owner) const;
	std::vector<const char*> GetRequiredExtensions(const Vulkan_Wrapper* owner) const;
	bool CheckExtensionSupport(const Vulkan_Wrapper* owner, const std::vector<const char*>& extensions) const;
};

