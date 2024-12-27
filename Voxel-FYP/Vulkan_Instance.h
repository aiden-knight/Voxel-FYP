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

	const vk::raii::Instance& GetHandle() const { return m_instance; }
private:
	vk::raii::Instance m_instance;

	vk::raii::Instance CreateInstance(const Vulkan_Wrapper* owner) const;
	std::vector<const char*> GetRequiredExtensions(const Vulkan_Wrapper* owner) const;
	bool CheckExtensionSupport(const Vulkan_Wrapper* owner, const std::vector<const char*>& extensions) const;
};

