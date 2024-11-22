#include "Vulkan_Instance.h"
#include "Vulkan_Wrapper.h"
#include "Window.h"

#include <set>
#include <exception>

struct Vulkan_Instance::CreateInfo
{
	vk::InstanceCreateInfo createInfo;
	vk::ApplicationInfo appInfo;
	std::vector<const char*> extensions;
	std::vector<const char*> validationLayers;
};

Vulkan_Instance::Vulkan_Instance(const Vulkan_Wrapper* owner) :
	m_instance{ owner->GetContext(), GetCreateInfo(owner).createInfo }
{

}

vk::raii::DebugUtilsMessengerEXT Vulkan_Instance::CreateDebugUtilMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT createInfo) const
{
	return m_instance.createDebugUtilsMessengerEXT(createInfo);
}

Vulkan_Instance::CreateInfo Vulkan_Instance::GetCreateInfo(const Vulkan_Wrapper* owner) const
{
	CreateInfo info;

	info.appInfo = vk::ApplicationInfo{
		"Voxel FYP",
		VK_MAKE_VERSION(1,0,0),
		"No Engine",
		VK_MAKE_VERSION(1,0,0),
		VK_API_VERSION_1_0
	};

	info.extensions = GetRequiredExtensions(owner);
	if (!CheckExtensionSupport(owner, info.extensions))
		throw std::runtime_error("extensions not supported");

	info.createInfo = vk::InstanceCreateInfo{
		vk::InstanceCreateFlags(),
		&info.appInfo,
		0, nullptr,
		static_cast<uint32_t>(info.extensions.size()), info.extensions.data()
	};

	if (owner->IsValidationEnabled())
	{
		info.validationLayers = owner->GetValidationLayers();
		info.createInfo.enabledLayerCount = static_cast<uint32_t>(info.validationLayers.size());
		info.createInfo.ppEnabledLayerNames = info.validationLayers.data();
	}
	return info;
}

std::vector<const char*> Vulkan_Instance::GetRequiredExtensions(const Vulkan_Wrapper* owner) const
{
	WindowExtensions windowExtensions = owner->GetWindow()->GetRequiredInstanceExtensions();
	std::vector<const char*> extensions{ windowExtensions.extensions, windowExtensions.extensions + windowExtensions.count };

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
