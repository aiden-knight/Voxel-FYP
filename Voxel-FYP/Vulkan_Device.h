#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <optional> 

#include "Vulkan_FWD.h"

enum QueueType
{
	GRAPHICS,
	PRESENT
};

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

class Vulkan_Device
{
public:
	Vulkan_Device(InstancePtr instance, SurfacePtr surface);

	vk::raii::Queue GetQueue(QueueType type) const;
	uint32_t GetQueueIndex(QueueType type) const;

	uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

	const vk::raii::Device& GetHandle() const { return m_device; }

	const SwapChainSupportDetails GetSwapChainSupportDetails() const { return m_swapChainSupportDetails; }

private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	} m_queueFamilies;

	SwapChainSupportDetails m_swapChainSupportDetails;

	vk::raii::PhysicalDevice m_physicalDevice;
	vk::raii::Device m_device;

	vk::raii::Device CreateDevice() const;

	vk::raii::PhysicalDevice ChoosePhysicalDevice(const std::unique_ptr<Vulkan_Instance>& instance, const std::unique_ptr<Vulkan_Surface>& surface);
};

