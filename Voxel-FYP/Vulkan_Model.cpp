#include "Vulkan_Model.h"
#include "Vulkan_Buffer.h"
#include "ObjectLoader.h"

template<typename DataType>
std::unique_ptr<Vulkan_Buffer> CreateSingleBuffer(DevicePtr device, CommandPoolPtr transferPool, const std::vector<DataType>& dataVector, vk::BufferUsageFlagBits usage)
{
	vk::DeviceSize bufferSize = sizeof(DataType) * dataVector.size();

	Vulkan_Buffer stagingBuffer{ device, bufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	stagingBuffer.FillBuffer(dataVector.data());

	std::unique_ptr<Vulkan_Buffer> buffer(new Vulkan_Buffer(device, bufferSize,
		vk::BufferUsageFlagBits::eTransferDst | usage,
		vk::MemoryPropertyFlagBits::eDeviceLocal));

	buffer->CopyFromBuffer(transferPool, stagingBuffer, bufferSize);
	return buffer;
}

Vulkan_Model::Vulkan_Model(DevicePtr device, CommandPoolPtr transferPool, std::string path) :
	m_mesh{ObjectLoader::LoadMesh(path)},
	m_vertexBuffer{ CreateSingleBuffer(device, transferPool, m_mesh.vertices, vk::BufferUsageFlagBits::eVertexBuffer) },
	m_indexBuffer{ CreateSingleBuffer(device, transferPool, m_mesh.indices, vk::BufferUsageFlagBits::eIndexBuffer) }
{

}

void Vulkan_Model::Draw(const vk::raii::CommandBuffer& commandBuffer)
{
	std::array<vk::Buffer, 1> vertexBuffers{ m_vertexBuffer->GetHandle() };
	std::array<vk::DeviceSize, 1> offsets{ 0 };
	commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(m_indexBuffer->GetHandle(), 0, vk::IndexType::eUint32);

	commandBuffer.drawIndexed(static_cast<uint32_t>(m_mesh.indices.size()), 1, 0, 0, 0);
}
