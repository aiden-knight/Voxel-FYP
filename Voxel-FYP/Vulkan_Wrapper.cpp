#include "Vulkan_Wrapper.h"
#include "Vulkan_Instance.h"
#include "Vulkan_Debugger.h"
#include "Vulkan_Surface.h"
#include "Vulkan_Device.h"

Vulkan_Wrapper::Vulkan_Wrapper(GLFW_Window* window, bool validationEnabled) :
	m_validationLayersEnabled(validationEnabled),
	m_ctx{}
{
	m_window = window;
	m_instance.reset(new Vulkan_Instance(this));
	m_debugMessenger.reset(new Vulkan_Debugger(m_instance));
	m_surface.reset(new Vulkan_Surface(m_instance, m_window));
	m_device.reset(new Vulkan_Device(m_instance, m_surface));
}

Vulkan_Wrapper::~Vulkan_Wrapper()
{
}

std::vector<const char*> Vulkan_Wrapper::GetValidationLayers() const
{
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	return validationLayers;
}
