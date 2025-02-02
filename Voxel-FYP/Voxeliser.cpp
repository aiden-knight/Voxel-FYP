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

Voxeliser::Voxeliser(DevicePtr device, CommandPoolPtr transferPool, Vulkan_Buffer* particlesOut, const Mesh& mesh, const VoxelisationUniform& voxelisationInfo)
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

	// creates the voxelisation compute pipeline
	m_voxelisationDescriptors.reset(new Vulkan_DescriptorSets(device, voxelisationDescriptors, 1));
	m_voxelisationPipeline.reset(new Vulkan_Pipeline(device, m_voxelisationDescriptors, "shaders/voxelisation.comp.spv"));

	// have to store indices in an alignment that fits with the storage buffer
	std::vector<Triangle> indices;
	indices.reserve(mesh.indices.size() / 3);
	for (int i = 0; i < mesh.indices.size(); i += 3)
	{
		Triangle triangle{
			mesh.indices[i],
			mesh.indices[i + 1],
			mesh.indices[i + 2]
		};
		indices.push_back(triangle);
	}

	// create uniform buffer
	std::vector<VoxelisationUniform> voxelUbo = { voxelisationInfo };
	m_voxelUBO = CreateSingleBuffer(device, transferPool, voxelUbo, vk::BufferUsageFlagBits::eUniformBuffer);
	m_vertices = CreateSingleBuffer(device, transferPool, mesh.vertices, vk::BufferUsageFlagBits::eStorageBuffer);
	m_indices = CreateSingleBuffer(device, transferPool, indices, vk::BufferUsageFlagBits::eStorageBuffer);

	// update descriptor sets with the data required for voxelisation
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
		sizeof(Triangle) * indices.size()
	}} };
	
	std::vector<vk::DescriptorBufferInfo> particleBuffer{ {{
		particlesOut->GetHandle(),
		0,
		sizeof(Voxel) * voxelisationInfo.voxelResolution * voxelisationInfo.voxelResolution * voxelisationInfo.voxelResolution
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

void Voxeliser::Voxelise(DevicePtr device, CommandPoolPtr computePool, size_t resolution)
{
	auto commandBuffer = computePool->BeginSingleTimeCommands();

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_voxelisationPipeline->GetHandle());
	const std::vector<vk::DescriptorSet> descriptorSets{ m_voxelisationDescriptors->GetDesciptorSet(0) };
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_voxelisationPipeline->GetLayout(), 0, descriptorSets, {});

	uint32_t dispatchCount = resolution / 8;
	if (resolution % 8 != 0)
		dispatchCount++;
	commandBuffer.dispatch(dispatchCount, dispatchCount, dispatchCount);

	computePool->EndSingleTimeCommands(std::move(commandBuffer));

	// wait for  the voxelisation to be complete
	device->GetHandle().waitIdle();
}
