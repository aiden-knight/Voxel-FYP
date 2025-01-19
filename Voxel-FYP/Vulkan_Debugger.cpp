#include "Vulkan_Debugger.h"
#include "Vulkan_Instance.h"

Vulkan_Debugger::Vulkan_Debugger(InstancePtr instance) :
	m_debugMessenger{ instance->CreateDebugUtilMessengerEXT(GetCreateInfo()) }
{

}

void Vulkan_Debugger::CheckVkResult(VkResult err)
{
	if (err == 0) return;

	std::cout << ("Vulkan Error: VkResult = " + err) << std::endl;

	if (err < 0) abort();
}

vk::DebugUtilsMessengerCreateInfoEXT Vulkan_Debugger::GetCreateInfo()
{
	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
	};
	vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
	};

	vk::DebugUtilsMessengerCreateInfoEXT createInfo{
		{}, // flags (use default)
		severityFlags, // message severity
		messageTypeFlags, // messageType
		VulkanMessengerUtilCallback, // callback
		{} // userdata
	};
	return createInfo;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan_Debugger::VulkanMessengerUtilCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
