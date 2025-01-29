#pragma once
#include "Vulkan_FWD.h"
#include "Structures.h"
#include "Mesh.h"

class Voxeliser
{
public:
	Voxeliser(DevicePtr device, CommandPoolPtr transferPool, Vulkan_Buffer* particlesOut, const Mesh& mesh, const VoxelisationUniform& voxelisationInfo);
	~Voxeliser();

private:
	std::unique_ptr<Vulkan_DescriptorSets> m_voxelisationDescriptors;
	std::unique_ptr<Vulkan_Pipeline> m_voxelisationPipeline;

	std::unique_ptr<Vulkan_Buffer> m_voxelUBO;
	std::unique_ptr<Vulkan_Buffer> m_vertices;
	std::unique_ptr<Vulkan_Buffer> m_indices;
};

