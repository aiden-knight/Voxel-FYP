#include "Voxeliser.h"
#include "Vulkan_Pipeline.h"
#include "Vulkan_DescriptorSets.h"
#include "Vulkan_Device.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_CommandPool.h"

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

Voxeliser::Voxeliser(DevicePtr device, CommandPoolPtr transferPool, Vulkan_Buffer* particlesOut, const Mesh& mesh, const VoxelisationUniform& voxelisationInfo) :
	m_commandBuffer{std::move(transferPool->CreateCommandBuffers(1).front())}
{
	std::vector<vk::DescriptorSetLayoutBinding> voxelisationDescriptors{ {
		{
			0, // binding
			vk::DescriptorType::eUniformBuffer,
			1, // descripter count
			vk::ShaderStageFlagBits::eCompute
		},
		{
			1,
			vk::DescriptorType::eStorageBuffer,
			1,
			vk::ShaderStageFlagBits::eCompute
		},
		{
			2, // binding
			vk::DescriptorType::eStorageBuffer,
			1, // descripter count
			vk::ShaderStageFlagBits::eCompute
		},
		{
			3, // binding
			vk::DescriptorType::eStorageBuffer,
			1, // descripter count
			vk::ShaderStageFlagBits::eCompute
		}
	} };

	// create the compute pipeline
	m_voxelisationDescriptors.reset(new Vulkan_DescriptorSets(device, voxelisationDescriptors, 1));
	m_voxelisationPipeline.reset(new Vulkan_Pipeline(device, m_voxelisationDescriptors, "shaders/voxelisation.comp.spv"));

	// create uniform buffer
	std::vector<VoxelisationUniform> voxelUbo = { voxelisationInfo };
	m_voxelUBO = CreateSingleBuffer(device, transferPool, voxelUbo, vk::BufferUsageFlagBits::eUniformBuffer);
	m_vertices = CreateSingleBuffer(device, transferPool, mesh.vertices, vk::BufferUsageFlagBits::eStorageBuffer);
	m_indices = CreateSingleBuffer(device, transferPool, mesh.indices, vk::BufferUsageFlagBits::eStorageBuffer);

	// update descriptor set
	std::vector<vk::DescriptorBufferInfo> bufferInfo{ {{
		m_voxelUBO->GetHandle(),
		0,
		sizeof(VoxelisationUniform)
	}} };

	std::vector<vk::DescriptorBufferInfo> verticesBuffer{ {{
		m_vertices->GetHandle(),
		0,
		sizeof(Vertex) * mesh.vertices.size()
	}} };

	std::vector<vk::DescriptorBufferInfo> indicesBuffer{ {{
		m_indices->GetHandle(),
		0,
		sizeof(uint32_t) * mesh.indices.size()
	}} };
	
	std::vector<vk::DescriptorBufferInfo> particleBuffer{ {{
		particlesOut->GetHandle(),
		0,
		sizeof(Particle) * voxelisationInfo.voxelResolution * voxelisationInfo.voxelResolution * voxelisationInfo.voxelResolution
	}} };

	std::vector<vk::WriteDescriptorSet> computeDescriptorWrites{ {
			{
				m_voxelisationDescriptors->GetDesciptorSet(0),
				0, 0, // dst binding and dst array element
				vk::DescriptorType::eUniformBuffer,
				{},
				bufferInfo
			},
			{
				m_voxelisationDescriptors->GetDesciptorSet(0),
				1, 0,
				vk::DescriptorType::eStorageBuffer,
				{},
				verticesBuffer
			},
			{
				m_voxelisationDescriptors->GetDesciptorSet(0),
				2, 0,
				vk::DescriptorType::eStorageBuffer,
				{},
				indicesBuffer
			},
			{
				m_voxelisationDescriptors->GetDesciptorSet(0),
				3, 0,
				vk::DescriptorType::eStorageBuffer,
				{},
				particleBuffer
			}
		} };

	device->GetHandle().updateDescriptorSets(computeDescriptorWrites, {});
}

Voxeliser::~Voxeliser()
{

}

void Voxeliser::Voxelise(DevicePtr device, size_t resolution)
{
	m_commandBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo{};
	m_commandBuffer.begin(beginInfo);

	m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_voxelisationPipeline->GetHandle());
	const std::vector<vk::DescriptorSet> descriptorSets{ m_voxelisationDescriptors->GetDesciptorSet(0) };
	m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_voxelisationPipeline->GetLayout(), 0, descriptorSets, {});

	uint32_t dispatchCount = resolution / 8;
	m_commandBuffer.dispatch(dispatchCount, dispatchCount, dispatchCount);

	m_commandBuffer.end();


	std::array<vk::CommandBuffer, 1> commandBuffers{ m_commandBuffer };
	std::array<vk::SubmitInfo, 1> submitInfo{ {{ {}, {}, commandBuffers, {}}} };
	device->GetQueue(GRAPHICS).submit(submitInfo);
	device->GetHandle().waitIdle();
}
