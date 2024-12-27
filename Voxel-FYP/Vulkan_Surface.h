#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>

class GLFW_Window;
#include "Vulkan_FWD.h"

class Vulkan_Surface
{
public:
	Vulkan_Surface(InstancePtr instance, const GLFW_Window* window);

	const vk::raii::SurfaceKHR& GetHandle() const { return m_surface; }
private:
	VkSurfaceKHR CreateSurface(InstancePtr instance, const GLFW_Window* window);

	vk::raii::SurfaceKHR m_surface;
};

