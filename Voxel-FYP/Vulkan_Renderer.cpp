#include "Vulkan_Renderer.h"
#include "Vulkan_CommandPool.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_SwapChain.h"
#include "Vulkan_Pipeline.h"
#include "Vulkan_Device.h"
#include "Vulkan_Wrapper.h"
#include "Vulkan_Buffer.h"
#include "GLFW_Window.h"
#include "Structures.h"

const std::vector<Vertex> g_vertices = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

Vulkan_Renderer::Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, SwapChainPtr swapChain, PipelinePtr pipeline, CommandPoolPtr graphicsPool) :
	m_owner{owner},
	m_deviceRef{device},
	m_swapChainRef{swapChain},
	m_renderPassRef{renderPass},
	m_pipelineRef{pipeline},
	m_commandBuffers{graphicsPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT)}
{
	m_clearColour = { {0.0f, 0.0f, 0.0f, 1.0f} };

	m_imageAvailableSemaphore.reserve(2);
	m_renderFinishedSemaphore.reserve(2);
	m_inFlightFence.reserve(2);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
	{
		m_imageAvailableSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_renderFinishedSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_inFlightFence.emplace_back(device->GetHandle(), vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	CreateVertexBuffer(device, graphicsPool);
}

Vulkan_Renderer::~Vulkan_Renderer()
{
	// ensure all commands are completed before destroying renderer
	m_deviceRef->GetHandle().waitIdle();
}

void Vulkan_Renderer::DrawFrame() 
{
	std::array<vk::Fence, 1> fences{ m_inFlightFence[currentFrame]};
	{ auto discard = m_deviceRef->GetHandle().waitForFences(fences, vk::True, UINT64_MAX); }

	std::pair<vk::Result, uint32_t> ret = m_swapChainRef->GetHandle().acquireNextImage(UINT64_MAX, m_imageAvailableSemaphore[currentFrame]);
	if (ret.first == vk::Result::eErrorOutOfDateKHR) {
		m_owner->RecreateSwapChain();
		return;
	} else if (ret.first != vk::Result::eSuccess && ret.first != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}

	m_deviceRef->GetHandle().resetFences(fences);
	m_commandBuffers[currentFrame].reset();
	RecordCommandBuffer(ret.second);

	// submit info
	std::array<vk::Semaphore, 1> waitSemaphores{ m_imageAvailableSemaphore[currentFrame] };
	std::array<vk::PipelineStageFlags, 1> waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::array<vk::CommandBuffer, 1> commandBuffers{ m_commandBuffers[currentFrame] };
	std::array<vk::Semaphore, 1> signalSemaphores{ m_renderFinishedSemaphore[currentFrame] };

	std::array<vk::SubmitInfo, 1> submitInfo{ {{ waitSemaphores, waitStages, commandBuffers, signalSemaphores}} };
	m_deviceRef->GetQueue(GRAPHICS).submit(submitInfo, m_inFlightFence[currentFrame]);

	// present info
	std::array<vk::SwapchainKHR, 1> swapchain{ m_swapChainRef->GetHandle() };

	vk::PresentInfoKHR presentInfo{
		signalSemaphores, swapchain, {ret.second}
	};

	// present image to screen
	vk::Result result = m_deviceRef->GetQueue(PRESENT).presentKHR(presentInfo);
	
	// check if swapchain needs updating
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_owner->GetWindow()->resized) {
		m_owner->GetWindow()->resized = false;
		m_owner->RecreateSwapChain();
	} else if (result != vk::Result::eSuccess) {
		throw std::runtime_error("failed to present swap chain image");
	}

	// move to next frame
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan_Renderer::RecordCommandBuffer(uint32_t imageIndex) 
{
	vk::CommandBufferBeginInfo beginInfo{};
	m_commandBuffers[currentFrame].begin(beginInfo);

	std::array<vk::ClearValue, 1> clearValues = { m_clearColour };

	vk::Rect2D renderArea{ {0,0}, m_swapChainRef->GetImageExtent()};

	vk::RenderPassBeginInfo renderPassInfo{
		m_renderPassRef->GetHandle(),
		m_swapChainRef->GetFramebuffer(imageIndex),// FRAMEBUFFER
		renderArea, // RENDER AREA
		clearValues
	};

	m_commandBuffers[currentFrame].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	m_commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetHandle());

	std::array<vk::Viewport, 1> viewports{ {{
		0.0f, 0.0f, static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height),// render area
		0.0f, 1.0f // min and max depth
	}} };
	std::array<vk::Rect2D, 1> scissors{ renderArea };
	m_commandBuffers[currentFrame].setViewport(0, viewports);
	m_commandBuffers[currentFrame].setScissor(0, scissors);

	std::array<vk::Buffer, 1> vertexBuffers{m_vertexBuffer->GetHandle()};
	std::array<vk::DeviceSize, 1> offsets{ 0 };
	m_commandBuffers[currentFrame].bindVertexBuffers(0, vertexBuffers, offsets);

	m_commandBuffers[currentFrame].draw(static_cast<uint32_t>(g_vertices.size()), 1, 0, 0);

	m_commandBuffers[currentFrame].endRenderPass();
	m_commandBuffers[currentFrame].end();
}

void Vulkan_Renderer::CreateVertexBuffer(DevicePtr device, CommandPoolPtr transferPool)
{
	vk::DeviceSize bufferSize = sizeof(Vertex) * g_vertices.size();

	Vulkan_Buffer stagingBuffer{ device, bufferSize, 
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	stagingBuffer.FillBuffer(g_vertices.data());

	m_vertexBuffer.reset(new Vulkan_Buffer(device, bufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal));
	// TRANSFER STAGING BUFFER MEMORY INTO VERTEX BUFFER
	m_vertexBuffer->CopyFromBuffer(transferPool, stagingBuffer, bufferSize);
}