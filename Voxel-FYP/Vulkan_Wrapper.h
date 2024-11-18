#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <vector>

class Window;
class Vulkan_Instance;
class Vulkan_Debugger;

class Vulkan_Wrapper
{
public:
	Vulkan_Wrapper(Window* window, bool validationEnabled = false);
	~Vulkan_Wrapper();

	inline bool IsValidationEnabled() const { return m_validationLayersEnabled; }
	inline const Window* GetWindow() const { return m_window; }
	inline const vk::raii::Context& GetContext() const { return m_ctx; }

	std::vector<const char*> GetValidationLayers() const;
private:
	const bool m_validationLayersEnabled;

	vk::raii::Context m_ctx;
	std::unique_ptr<Vulkan_Instance> m_instance;
	std::unique_ptr<Vulkan_Debugger> m_debugMessenger;

	Window* m_window;

};