#pragma once
#include "Vulkan_FWD.h"
#include "Mesh.h"

class Vulkan_Model
{
public:
	Vulkan_Model(DevicePtr device, CommandPoolPtr transferPool, const std::string path);

	void Draw(const vk::raii::CommandBuffer& commandBuffer);
private:
	Mesh m_mesh;
	std::unique_ptr<Vulkan_Buffer> m_vertexBuffer;
	std::unique_ptr<Vulkan_Buffer> m_indexBuffer;
};

