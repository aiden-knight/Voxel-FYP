#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <optional> 
#include "Vulkan_FWD.h"

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class Vulkan_Wrapper;

class Vulkan_Renderer
{
public:
	Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, SwapChainPtr swapChain, PipelinePtr pipeline, CommandPoolPtr graphicsPool);
	~Vulkan_Renderer();

	void DrawFrame();
private:
	// references to needed elements
	Vulkan_Wrapper *const m_owner;
	DevicePtr m_deviceRef;
	RenderPassPtr m_renderPassRef;
	SwapChainPtr m_swapChainRef;
	PipelinePtr m_pipelineRef;

	// for actual rendering
	vk::raii::CommandBuffers m_commandBuffers;
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphore;
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphore;
	std::vector<vk::raii::Fence> m_inFlightFence;

	uint32_t currentFrame = 0;
	vk::ClearValue m_clearColour;

	std::unique_ptr<Vulkan_Buffer> m_vertexBuffer;

	void RecordCommandBuffer(uint32_t imageIndex);
	void CreateVertexBuffer(DevicePtr device, CommandPoolPtr transferPool);
};