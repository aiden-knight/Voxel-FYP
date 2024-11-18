#include "Vulkan_Wrapper.h"
#include "Vulkan_Instance.h"
#include "Vulkan_Debugger.h"

Vulkan_Wrapper::Vulkan_Wrapper(Window* window, bool validationEnabled) :
	m_validationLayersEnabled(validationEnabled),
	m_ctx{}
{
	m_window = window;
	m_instance.reset(new Vulkan_Instance(this));
	m_debugMessenger.reset(new Vulkan_Debugger(m_instance));
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
