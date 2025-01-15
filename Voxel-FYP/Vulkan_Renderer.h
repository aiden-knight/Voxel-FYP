#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <optional> 
#include "Vulkan_FWD.h"

class Vulkan_Wrapper;

class Vulkan_Renderer
{
public:
	Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, SwapChainPtr swapChain, PipelinePtr pipeline, CommandPoolPtr graphicsPool, DescriptorSetsPtr descriptorSets);
	~Vulkan_Renderer();

	void DrawFrame();
private:
	using Buffer = std::unique_ptr<Vulkan_Buffer>;

	// references to needed elements
	Vulkan_Wrapper *const m_owner;
	DevicePtr m_deviceRef;
	RenderPassPtr m_renderPassRef;
	SwapChainPtr m_swapChainRef;
	PipelinePtr m_pipelineRef;
	DescriptorSetsPtr m_descriptorSetsRef;

	// for actual rendering
	vk::raii::CommandBuffers m_commandBuffers;
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphore;
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphore;
	std::vector<vk::raii::Fence> m_inFlightFence;

	uint32_t currentFrame = 0;
	vk::ClearValue m_clearColour;

	std::unique_ptr<Vulkan_Model> m_model;
	std::vector<std::pair<Vulkan_Buffer, void*>> m_uniformBuffers;

	void RecordCommandBuffer(uint32_t imageIndex);
	void CreateUniformBuffer(DevicePtr device);

	void UpdateUniforms(uint32_t imageIndex);
};