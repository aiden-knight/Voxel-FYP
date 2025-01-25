#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <optional> 
#include "Vulkan_FWD.h"

#include "Mesh.h"

class Vulkan_Wrapper;

class Vulkan_Renderer
{
public:
	Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, RenderPassPtr imGuiRenderPass, SwapChainPtr swapChain, 
		PipelinePtr pipeline, CommandPoolPtr graphicsPool, DescriptorSetsPtr descriptorSets, PipelinePtr computePipeline, DescriptorSetsPtr computeDescriptors);
	~Vulkan_Renderer();

	void DrawFrame();
private:
	using Buffer = std::unique_ptr<Vulkan_Buffer>;

	// references to needed elements
	Vulkan_Wrapper *const m_owner;
	DevicePtr m_deviceRef;
	RenderPassPtr m_renderPassRef;
	RenderPassPtr m_imGuiRenderPassRef;
	SwapChainPtr m_swapChainRef;
	PipelinePtr m_pipelineRef;
	DescriptorSetsPtr m_descriptorSetsRef;

	PipelinePtr m_computePipelineRef;
	DescriptorSetsPtr m_computeDescriptorSetsRef;

	CommandPoolPtr m_graphicsPoolRef;

	// for actual rendering
	vk::raii::CommandBuffers m_commandBuffers;
	vk::raii::CommandBuffers m_imguiCommandBuffers;
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphore;
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphore;
	std::vector<vk::raii::Fence> m_inFlightFence;

	vk::raii::CommandBuffers m_computeCommandBuffers;
	std::vector<vk::raii::Semaphore> m_computeFinishedSemaphore;
	std::vector<vk::raii::Fence> m_computeInFlightFence;


	uint32_t m_currentFrame = 0;
	vk::ClearValue m_clearColour;

	Mesh m_mesh;
	std::unique_ptr<Vulkan_Model> m_model;
	std::vector<std::pair<Vulkan_Buffer, void*>> m_uniformBuffers;
	std::vector<Vulkan_Buffer> m_computeStorageBuffers;

	void RecordComputeCommands();
	void RecordCommandBuffer(uint32_t imageIndex);
	void RecordImGuiCommandBuffer(uint32_t imageIndex);

	void CentreMesh(Mesh& mesh);

	void CreateUniformBuffer(DevicePtr device);
	void CreateComputeStorageBuffers(DevicePtr device, CommandPoolPtr graphicsPool);
	void CreateFrameData(DevicePtr device);

	void ClearRenderer(DevicePtr device);
	void CreateRenderer(DevicePtr device, CommandPoolPtr graphicsPool);

	void UpdateUniforms(uint32_t imageIndex);

	bool m_runCompute = false;
	float m_velocityMult = 50.0f;

	glm::vec3 m_cameraPos = glm::vec3(0.0f, 3.5f, 7.0f);
	glm::vec3 m_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	
	std::string m_modelString = "teapot";
	float m_modelHalfExtent = 2;
	int m_modelResolution = 10;
	float m_voxelHalfExtent = m_modelHalfExtent / m_modelResolution;
	size_t m_particleCount;

	void DrawImGui();
};