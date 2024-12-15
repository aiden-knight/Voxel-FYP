#include "Vulkan_Surface.h"
#include "GLFW_Window.h"
#include "Vulkan_Instance.h"

#include <GLFW/glfw3.h>

Vulkan_Surface::Vulkan_Surface(const std::unique_ptr<Vulkan_Instance>& instance, const GLFW_Window* window)
	: m_surface(instance->GetHandle(), CreateSurface(instance, window))
{
	
}

VkSurfaceKHR Vulkan_Surface::CreateSurface(const std::unique_ptr<Vulkan_Instance>& instance, const GLFW_Window* window)
{
	VkSurfaceKHR surface;
	glfwCreateWindowSurface(*instance->GetHandle(), window->GetHandle(), nullptr, &surface);
	return surface;
}
