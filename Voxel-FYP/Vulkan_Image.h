#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include "Vulkan_FWD.h"

class Vulkan_Image
{
public:
	Vulkan_Image(DevicePtr device, vk::Extent3D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags memoryProps, vk::ImageAspectFlags aspectFlags);

	const vk::raii::ImageView& GetImageView() const { return m_imageView; }
private:
	vk::raii::Image m_image;
	vk::raii::DeviceMemory m_imageMemory;
	vk::raii::ImageView m_imageView;

	vk::raii::Image CreateImage(DevicePtr device, vk::Extent3D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags memoryProps);
	vk::raii::DeviceMemory AllocateMemory(DevicePtr device, vk::MemoryPropertyFlags memoryProps);
	vk::raii::ImageView CreateImageView(DevicePtr device, vk::Format format, vk::ImageAspectFlags aspectFlags);
};

