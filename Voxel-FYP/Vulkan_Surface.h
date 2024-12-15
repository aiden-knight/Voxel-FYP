#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>

class GLFW_Window;
class Vulkan_Instance;

class Vulkan_Surface
{
public:
	Vulkan_Surface(const std::unique_ptr<Vulkan_Instance>& instance, const GLFW_Window* window);

	const vk::raii::SurfaceKHR& GetHandle() const { return m_surface; }
private:
	VkSurfaceKHR CreateSurface(const std::unique_ptr<Vulkan_Instance>& instance, const GLFW_Window* window);

	vk::raii::SurfaceKHR m_surface;
};

