#include "Vulkan_Instance.h"
#include "Vulkan_Wrapper.h"
#include "GLFW_Window.h"

#include <set>
#include <exception>

Vulkan_Instance::Vulkan_Instance(const Vulkan_Wrapper* owner) :
	m_instance{ CreateInstance(owner) }
{

}

vk::raii::DebugUtilsMessengerEXT Vulkan_Instance::CreateDebugUtilMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT createInfo) const
{
	return m_instance.createDebugUtilsMessengerEXT(createInfo);
}

vk::raii::Instance Vulkan_Instance::CreateInstance(const Vulkan_Wrapper* owner) const
{
	vk::ApplicationInfo appInfo {
		"Voxel FYP",
		VK_MAKE_VERSION(1,0,0),
		"No Engine",
		VK_MAKE_VERSION(1,0,0),
		VK_API_VERSION_1_0
	};

	std::vector<const char*> extensions = GetRequiredExtensions(owner);
	if (!CheckExtensionSupport(owner, extensions))
		throw std::runtime_error("extensions not supported");

	std::vector<const char*> validationLayers;
	if (owner->IsValidationEnabled()) {
		validationLayers = owner->GetValidationLayers();
	}

	vk::InstanceCreateInfo createInfo{
		{}, // flags
		&appInfo,
		validationLayers,
		extensions
	};
	
	return owner->GetContext().createInstance(createInfo);
}

std::vector<const char*> Vulkan_Instance::GetRequiredExtensions(const Vulkan_Wrapper* owner) const
{
	std::vector<const char*> extensions = owner->GetWindow()->GetRequiredInstanceExtensions();

	if (owner->IsValidationEnabled())
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	
	return extensions;
}

bool Vulkan_Instance::CheckExtensionSupport(const Vulkan_Wrapper* owner, const std::vector<const char*>& extensions) const
{
	std::vector<vk::ExtensionProperties> supportedExtensions = owner->GetContext().enumerateInstanceExtensionProperties();

	std::set<std::string> wanted{ extensions.begin(), extensions.end() };
	for (const auto& supported : supportedExtensions)
	{
		wanted.erase(supported.extensionName);
	}
	return wanted.empty();
}
