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


Vulkan_Device::Vulkan_Device(InstancePtr instance, SurfacePtr surface) :
	m_physicalDevice{ChoosePhysicalDevice(instance, surface)},
	m_device{CreateDevice()}
{

}

vk::raii::Queue Vulkan_Device::GetQueue(QueueType type) const
{
	return m_device.getQueue(GetQueueIndex(type), 0);
}

uint32_t Vulkan_Device::GetQueueIndex(QueueType type) const 
{
	switch (type) {
	case GRAPHICS:
		return m_queueFamilies.graphicsFamily.value();
	case PRESENT:
		return m_queueFamilies.presentFamily.value();
	default:
		throw std::runtime_error("queue type not supported");
	}
}

uint32_t Vulkan_Device::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const 
{
	vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}

vk::Format Vulkan_Device::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags) const
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties properties = m_physicalDevice.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & featureFlags) == featureFlags)
			return format;
		else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
			return format;
	}
	throw std::runtime_error("failed to find supported format");
}

void Vulkan_Device::ResetSwapChainSupportDetails(SurfacePtr surface)
{
	m_swapChainSupportDetails.capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(surface->GetHandle());
}

vk::raii::Device Vulkan_Device::CreateDevice() const
{
	std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilies.graphicsFamily.value() };
	float queuePriority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = vk::True;

	// get all device queue create infos
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo(
			{}, // flags
			queueFamily,
			1, // count
			&queuePriority
			));
	}
	 
	vk::DeviceCreateInfo createInfo = {
		{}, // flags
		static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(),
		0, {}, // validation layers
		static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
		&deviceFeatures
	};

	return m_physicalDevice.createDevice(createInfo);
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