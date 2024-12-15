#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <vector>

class GLFW_Window;
class Vulkan_Instance;
class Vulkan_Debugger;
class Vulkan_Device;
class Vulkan_Surface;

class Vulkan_Wrapper
{
public:
	Vulkan_Wrapper(GLFW_Window* window, bool validationEnabled = false);
	~Vulkan_Wrapper(); // must be included in CPP to use fwd declare in unique_ptr

	inline bool IsValidationEnabled() const { return m_validationLayersEnabled; }
	inline const GLFW_Window* GetWindow() const { return m_window; }
	inline const vk::raii::Context& GetContext() const { return m_ctx; }

	std::vector<const char*> GetValidationLayers() const;
private:
	const bool m_validationLayersEnabled;

	vk::raii::Context m_ctx;
	std::unique_ptr<Vulkan_Instance> m_instance;
	std::unique_ptr<Vulkan_Debugger> m_debugMessenger;
	std::unique_ptr<Vulkan_Surface> m_surface;
	std::unique_ptr<Vulkan_Device> m_device;

	GLFW_Window* m_window;
};