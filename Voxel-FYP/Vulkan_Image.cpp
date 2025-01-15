#include "Vulkan_Image.h"
#include "Vulkan_Device.h"

Vulkan_Image::Vulkan_Image(DevicePtr device, vk::Extent3D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags memoryProps, vk::ImageAspectFlags aspectFlags) :
	m_image{ CreateImage(device, extent, format, tiling, usage, memoryProps) },
	m_imageMemory{ AllocateMemory(device, memoryProps) },
	m_imageView{ CreateImageView(device, format, aspectFlags) }
{
	
}

vk::raii::Image Vulkan_Image::CreateImage(DevicePtr device, vk::Extent3D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags memoryProps)
{	
	vk::ImageCreateInfo createInfo{
		{},
		vk::ImageType::e2D,
		format, extent,
		1, 1, // mip levels and array layers
		vk::SampleCountFlagBits::e1,
		tiling, usage,
		vk::SharingMode::eExclusive,
	};

	return device->GetHandle().createImage(createInfo);
}

vk::raii::DeviceMemory Vulkan_Image::AllocateMemory(DevicePtr device, vk::MemoryPropertyFlags memoryProps)
{
	vk::MemoryRequirements memRequirements = m_image.getMemoryRequirements();

	vk::MemoryAllocateInfo allocInfo{
		memRequirements.size,
		device->FindMemoryType(memRequirements.memoryTypeBits, memoryProps)
	};

	return device->GetHandle().allocateMemory(allocInfo);
}

vk::raii::ImageView Vulkan_Image::CreateImageView(DevicePtr device, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
	m_image.bindMemory(m_imageMemory, 0);

	vk::ImageViewCreateInfo createInfo{
		{},
		m_image,
		vk::ImageViewType::e2D,
		format,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1)
	};

	return device->GetHandle().createImageView(createInfo);
}
