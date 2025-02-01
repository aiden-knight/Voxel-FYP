#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <optional> 
#include "Vulkan_FWD.h"

#include "ImGuiConfig.h"
#include "Mesh.h"

class Vulkan_Wrapper;
struct Particle;

class Vulkan_Renderer
{
public:
	Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, RenderPassPtr imGuiRenderPass, SwapChainPtr swapChain, 
		PipelinePtr pipeline, CommandPoolPtr graphicsPool, DescriptorSetsPtr descriptorSets);
	~Vulkan_Renderer();

	void DrawFrame();
	std::vector<Particle>& GetParticleVector() { return m_voxelisation; }
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

	CommandPoolPtr m_graphicsPoolRef;

	// for actual rendering
	vk::raii::CommandBuffers m_commandBuffers;
	vk::raii::CommandBuffers m_imguiCommandBuffers;
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphore;
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphore;
	std::vector<vk::raii::Fence> m_inFlightFence;

	uint32_t m_currentFrame = 0;
	vk::ClearValue m_clearColour;

	Mesh m_mesh;
	std::unique_ptr<Vulkan_Model> m_model;
	std::vector<std::pair<Vulkan_Buffer, void*>> m_uniformBuffers;

	std::vector<Particle> m_voxelisation;
	std::vector<std::pair<Vulkan_Buffer, void*>> m_vertexBuffers;

	void RecordCommandBuffer(uint32_t imageIndex);
	void RecordImGuiCommandBuffer(uint32_t imageIndex);

	void CentreMesh(Mesh& mesh);

	void CreateUniformBuffer(DevicePtr device);
	void CreateVoxelisationBuffers(DevicePtr device, CommandPoolPtr graphicsPool);
	void CreateFrameData(DevicePtr device);

	void CreateRenderer(DevicePtr device, CommandPoolPtr graphicsPool);

	void UpdateUniforms(uint32_t imageIndex);
	void UpdateVertexBuffer(uint32_t currentFrame);

	float m_voxelHalfExtent = ImGuiConfig::GetInstance()->modelHalfExtent / ImGuiConfig::GetInstance()->modelResolution;

	void DrawImGui();
};