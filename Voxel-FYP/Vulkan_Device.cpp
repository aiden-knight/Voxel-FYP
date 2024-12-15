#include "Vulkan_Device.h"

#include "Vulkan_Instance.h"
#include "Vulkan_Surface.h"

#include <exception>
#include <vector>
#include <string>
#include <set>

const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vulkan_Device::CreateInfo 
{
	float queuePriority;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	vk::PhysicalDeviceFeatures deviceFeatures;
	vk::DeviceCreateInfo createInfo;
};

Vulkan_Device::Vulkan_Device(const std::unique_ptr<Vulkan_Instance>& instance, const std::unique_ptr<Vulkan_Surface>& surface) :
	m_physicalDevice{ChoosePhysicalDevice(instance, surface)},
	m_device{m_physicalDevice, GetCreateInfo().createInfo, nullptr}
{

}

vk::raii::Queue Vulkan_Device::GetQueue(QueueType type) const
{
	switch (type)
	{
	case GRAPHICS:
		return m_device.getQueue(m_queueFamilies.graphicsFamily.value(), 0);
	case PRESENT:
		return m_device.getQueue(m_queueFamilies.presentFamily.value(), 0);
	default:
		throw std::runtime_error("queue type not supported");
	}
}

const Vulkan_Device::CreateInfo Vulkan_Device::GetCreateInfo() const
{
	CreateInfo createInfo;

	std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilies.graphicsFamily.value() };
	createInfo.queuePriority = 1.0f;

	// get all device queue create infos
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{
			{}, // flags
			queueFamily,
			1, // count
			&createInfo.queuePriority
		};

		createInfo.queueCreateInfos.push_back(queueCreateInfo);
	}
	 
	createInfo.createInfo = { 
		{}, // flags
		static_cast<uint32_t>(createInfo.queueCreateInfos.size()), createInfo.queueCreateInfos.data(),
		0, {}, // validation layers
		static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
		&createInfo.deviceFeatures
	};

	return createInfo;
}

vk::raii::PhysicalDevice Vulkan_Device::ChoosePhysicalDevice(const std::unique_ptr<Vulkan_Instance>& instance, const std::unique_ptr<Vulkan_Surface>& surface)
{
	// get all physical devices
	vk::raii::PhysicalDevices availableDevices(instance->GetHandle());
	uint32_t count = availableDevices.size();

	// loop through devices
	for (uint32_t index = 0; index < count; ++index)
	{
		const vk::raii::PhysicalDevice& device = availableDevices[index];

		// GET QUEUE FAMILIES
		bool queueFamiliesFound = false;
		m_queueFamilies = QueueFamilyIndices();
		uint32_t queueIndex = 0;
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
		for (const vk::QueueFamilyProperties& family : queueFamilies)
		{
			if (family.queueFlags & vk::QueueFlagBits::eGraphics)
				m_queueFamilies.graphicsFamily = queueIndex;
			
			if (device.getSurfaceSupportKHR(queueIndex, surface->GetHandle()))
				m_queueFamilies.presentFamily = queueIndex;

			queueFamiliesFound = m_queueFamilies.IsComplete();
			if (queueFamiliesFound) break;
			++queueIndex;
		}
		if (!queueFamiliesFound) continue; // if device is unsuitable continue

		// CHECK DEVICE EXTENSIONS
		std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
		std::set<std::string> requiredExtensions{ deviceExtensions.begin(), deviceExtensions.end() };
		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		if (!requiredExtensions.empty()) continue; // if device is unsuitable continue

		// CHECK SWAPCHAIN SUPPORT
		m_swapChainSupportDetails = SwapChainSupportDetails();
		m_swapChainSupportDetails.capabilities = device.getSurfaceCapabilitiesKHR(surface->GetHandle());
		m_swapChainSupportDetails.formats = device.getSurfaceFormatsKHR(surface->GetHandle());
		m_swapChainSupportDetails.presentModes = device.getSurfacePresentModesKHR(surface->GetHandle());
		if (m_swapChainSupportDetails.formats.empty() || m_swapChainSupportDetails.presentModes.empty()) continue;

		// if device suitable return it
		return availableDevices[index];
	}

	throw std::runtime_error("no supported physical devices available");
}