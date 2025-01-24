#include "Vulkan_Renderer.h"
#include "Vulkan_CommandPool.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_SwapChain.h"
#include "Vulkan_Pipeline.h"
#include "Vulkan_Device.h"
#include "Vulkan_Wrapper.h"
#include "Vulkan_Buffer.h"
#include "Vulkan_Model.h"
#include "Vulkan_DescriptorSets.h"
#include "ObjectLoader.h"
#include "GLFW_Window.h"
#include "Structures.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

#include <random>
#include <chrono>

constexpr uint32_t PARTICLE_COUNT = 4096;
Mesh g_mesh;

Vulkan_Renderer::Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, RenderPassPtr imGuiRenderPass, SwapChainPtr swapChain,
	PipelinePtr pipeline, CommandPoolPtr graphicsPool, DescriptorSetsPtr descriptorSets, PipelinePtr computePipeline, DescriptorSetsPtr computeDescriptors) :
	m_owner{owner},
	m_deviceRef{device},
	m_swapChainRef{swapChain},
	m_renderPassRef{renderPass},
	m_imGuiRenderPassRef{imGuiRenderPass},
	m_pipelineRef{pipeline},
	m_descriptorSetsRef{descriptorSets},
	m_computePipelineRef{ computePipeline },
	m_computeDescriptorSetsRef{ computeDescriptors },
	m_commandBuffers{graphicsPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT)},
	m_computeCommandBuffers{graphicsPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT)},
	m_imguiCommandBuffers{graphicsPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT)}
{
	m_clearColour = { {0.0f, 0.0f, 0.0f, 1.0f} };

	CreateComputeStorageBuffers(device, graphicsPool);

	m_imageAvailableSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
	m_computeFinishedSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFence.reserve(MAX_FRAMES_IN_FLIGHT);
	m_computeInFlightFence.reserve(MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
	{
		// create sync objects
		m_imageAvailableSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_renderFinishedSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_computeFinishedSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_inFlightFence.emplace_back(device->GetHandle(), vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		m_computeInFlightFence.emplace_back(device->GetHandle(), vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));

		// create uniform buffer
		CreateUniformBuffer(device);

		// update descriptor set with uniform buffer
		std::vector<vk::DescriptorBufferInfo> bufferInfo{ {{
			m_uniformBuffers[i].first.GetHandle(),
			0,
			sizeof(UniformBufferObject)
		}} };

		std::vector<vk::WriteDescriptorSet> descriptorWrites{ {{
			m_descriptorSetsRef->GetDesciptorSet(i),
			0, 0, // dst binding and dst array element
			vk::DescriptorType::eUniformBuffer,
			{},
			bufferInfo
		}} };

		device->GetHandle().updateDescriptorSets(descriptorWrites, {});

		std::vector<vk::DescriptorBufferInfo> storageBufferInfoLastFrame{ {{
			m_computeStorageBuffers[(i - 1) % MAX_FRAMES_IN_FLIGHT].GetHandle(),
			0,
			sizeof(Particle) * g_mesh.vertices.size()
		}} };

		std::vector<vk::DescriptorBufferInfo> storageBufferInfoCurrentFrame{ {{
			m_computeStorageBuffers[i].GetHandle(),
			0,
			sizeof(Particle) * g_mesh.vertices.size()
		}} };

		std::vector<vk::WriteDescriptorSet> computeDescriptorWrites{ {
			{
				m_computeDescriptorSetsRef->GetDesciptorSet(i),
				0, 0, // dst binding and dst array element
				vk::DescriptorType::eUniformBuffer,
				{},
				bufferInfo
			},
			{
				m_computeDescriptorSetsRef->GetDesciptorSet(i),
				1, 0,
				vk::DescriptorType::eStorageBuffer,
				{},
				storageBufferInfoLastFrame
			},
			{
				m_computeDescriptorSetsRef->GetDesciptorSet(i),
				2, 0,
				vk::DescriptorType::eStorageBuffer,
				{},
				storageBufferInfoCurrentFrame
			}
		} };
		device->GetHandle().updateDescriptorSets(computeDescriptorWrites, {});
	}
}

Vulkan_Renderer::~Vulkan_Renderer()
{
}

void Vulkan_Renderer::DrawFrame() 
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DrawImGui();

	ImGui::Render();

	// compute pipeline pass
	if (m_runCompute)
	{
		std::array<vk::Fence, 1> computeFences{ m_computeInFlightFence[m_currentFrame] };
		{ auto discard = m_deviceRef->GetHandle().waitForFences(computeFences, vk::True, UINT64_MAX); }

		m_deviceRef->GetHandle().resetFences(computeFences);
		m_computeCommandBuffers[m_currentFrame].reset();
		RecordComputeCommands();
	}

	UpdateUniforms(m_currentFrame);

	if (m_runCompute)
	{
		// compute submit info
		std::array<vk::Semaphore, 1> computeSignalSemaphores{ m_computeFinishedSemaphore[m_currentFrame] };
		std::array<vk::CommandBuffer, 1> computeCommandBuffers{ m_computeCommandBuffers[m_currentFrame] };
		std::array<vk::SubmitInfo, 1> computeSubmitInfo{ {{ {}, {}, computeCommandBuffers, computeSignalSemaphores}} };

		m_deviceRef->GetQueue(GRAPHICS).submit(computeSubmitInfo, m_computeInFlightFence[m_currentFrame]);
	}
	
	// graphics pipeline pass
	std::array<vk::Fence, 1> fences{ m_inFlightFence[m_currentFrame] };
	{ auto discard = m_deviceRef->GetHandle().waitForFences(fences, vk::True, UINT64_MAX); }

	std::pair<vk::Result, uint32_t> ret = m_swapChainRef->GetHandle().acquireNextImage(UINT64_MAX, m_imageAvailableSemaphore[m_currentFrame]);
	if (ret.first == vk::Result::eErrorOutOfDateKHR) {
		m_owner->RecreateSwapChain();
		return;
	}
	else if (ret.first != vk::Result::eSuccess && ret.first != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}

	m_deviceRef->GetHandle().resetFences(fences);
	m_commandBuffers[m_currentFrame].reset();
	RecordCommandBuffer(ret.second);
	
	m_imguiCommandBuffers[m_currentFrame].reset();
	RecordImGuiCommandBuffer(ret.second);

	// submit info
	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStages;
	if (m_runCompute)
	{
		waitSemaphores = { m_computeFinishedSemaphore[m_currentFrame], m_imageAvailableSemaphore[m_currentFrame] };
		waitStages = { vk::PipelineStageFlagBits::eVertexInput, vk::PipelineStageFlagBits::eColorAttachmentOutput };
	}
	else
	{
		waitSemaphores = { m_imageAvailableSemaphore[m_currentFrame] };
		 waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	}
	
	std::vector<vk::CommandBuffer> commandBuffers{ m_commandBuffers[m_currentFrame], m_imguiCommandBuffers[m_currentFrame] };
	std::array<vk::Semaphore, 1> signalSemaphores{ m_renderFinishedSemaphore[m_currentFrame] };

	std::array<vk::SubmitInfo, 1> submitInfo{ {{ waitSemaphores, waitStages, commandBuffers, signalSemaphores}} };
	m_deviceRef->GetQueue(GRAPHICS).submit(submitInfo, m_inFlightFence[m_currentFrame]);

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
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Vulkan_Renderer::RecordComputeCommands()
{
	vk::CommandBufferBeginInfo beginInfo{};
	m_computeCommandBuffers[m_currentFrame].begin(beginInfo);

	m_computeCommandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eCompute, m_computePipelineRef->GetHandle());
	const std::vector<vk::DescriptorSet> descriptorSets{ m_computeDescriptorSetsRef->GetDesciptorSet(m_currentFrame) };
	m_computeCommandBuffers[m_currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_computePipelineRef->GetLayout(), 0, descriptorSets, {});

	uint32_t dispatchCount = g_mesh.vertices.size() / 256;
	if (g_mesh.vertices.size() % 256 != 0)
		dispatchCount++;
	m_computeCommandBuffers[m_currentFrame].dispatch(dispatchCount, 1, 1);

	m_computeCommandBuffers[m_currentFrame].end();
}

void Vulkan_Renderer::RecordCommandBuffer(uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo beginInfo{};
	m_commandBuffers[m_currentFrame].begin(beginInfo);

	std::array<vk::ClearValue, 2> clearValues = { m_clearColour, {{1.0f, 0}} };

	vk::Rect2D renderArea{ {0,0}, m_swapChainRef->GetImageExtent()};

	vk::RenderPassBeginInfo renderPassInfo{
		m_renderPassRef->GetHandle(),
		m_swapChainRef->GetFramebuffer(imageIndex),// FRAMEBUFFER
		renderArea, // RENDER AREA
		clearValues
	};

	m_commandBuffers[m_currentFrame].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetHandle());

	std::array<vk::Viewport, 1> viewports{ {{
		0.0f, 0.0f, static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height),// render area
		0.0f, 1.0f // min and max depth
	}} };
	std::array<vk::Rect2D, 1> scissors{ renderArea };
	m_commandBuffers[m_currentFrame].setViewport(0, viewports);
	m_commandBuffers[m_currentFrame].setScissor(0, scissors);

	const std::vector<vk::DescriptorSet> descriptorSets{ m_descriptorSetsRef->GetDesciptorSet(m_currentFrame) };
	m_commandBuffers[m_currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetLayout(), 0, descriptorSets, {});

	std::array<vk::Buffer, 1> vertexBuffers{ m_computeStorageBuffers[m_currentFrame].GetHandle() };
	std::array<vk::DeviceSize, 1> offsets{ 0 };
	m_commandBuffers[m_currentFrame].bindVertexBuffers(0, vertexBuffers, offsets);
	m_commandBuffers[m_currentFrame].draw(g_mesh.vertices.size(), 1, 0, 0);

	m_commandBuffers[m_currentFrame].endRenderPass();
	m_commandBuffers[m_currentFrame].end();
}

void Vulkan_Renderer::RecordImGuiCommandBuffer(uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo beginInfo{};
	m_imguiCommandBuffers[m_currentFrame].begin(beginInfo);

	std::array<vk::ClearValue, 1> clearValues = { m_clearColour };
	vk::Rect2D renderArea{ {0,0}, m_swapChainRef->GetImageExtent() };
	vk::RenderPassBeginInfo renderPassInfo{
		m_imGuiRenderPassRef->GetHandle(),
		m_swapChainRef->GetImGuiFramebuffer(imageIndex),// FRAMEBUFFER
		renderArea, // RENDER AREA
		clearValues
	};

	m_imguiCommandBuffers[m_currentFrame].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *m_imguiCommandBuffers[m_currentFrame]);
	
	m_imguiCommandBuffers[m_currentFrame].endRenderPass();
	m_imguiCommandBuffers[m_currentFrame].end();
}

void Vulkan_Renderer::CreateUniformBuffer(DevicePtr device)
{
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	auto& [buffer, memory] = m_uniformBuffers.emplace_back(
		Vulkan_Buffer(device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
		nullptr);

	memory = buffer.MapMemory();
}

void Vulkan_Renderer::CreateComputeStorageBuffers(DevicePtr device, CommandPoolPtr graphicsPool)
{
	std::random_device rand{};
	std::mt19937_64 engine{rand()};
	std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };

	g_mesh = ObjectLoader::LoadMesh("stanford-bunny.obj");
	auto extent = m_swapChainRef->GetImageExtent();

	std::vector<Particle> particles;
	particles.reserve(g_mesh.vertices.size());
	for (const auto& vertex : g_mesh.vertices)
	{
		Particle particle;

		particle.position = glm::vec4(vertex.pos, 0.0f);
		particle.position *= 15;
		particle.velocity = glm::vec4(vertex.normal * 0.001f, 0.0f);
		particle.colour = glm::vec4(vertex.normal, 1.0f);
		particles.push_back(particle);
	}

	vk::DeviceSize bufferSize = sizeof(Particle) * particles.size();

	Vulkan_Buffer stagingBuffer{ device, bufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	stagingBuffer.FillBuffer(particles.data());

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		auto& buffer = m_computeStorageBuffers.emplace_back(device, bufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		buffer.CopyFromBuffer(graphicsPool, stagingBuffer, bufferSize);
	}
	
}

void Vulkan_Renderer::UpdateUniforms(uint32_t imageIndex)
{
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;

	vk::Extent2D extent = m_swapChainRef->GetImageExtent();

	UniformBufferObject ubo{};
	ubo.view = glm::lookAt(m_cameraPos, m_cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / static_cast<float>(extent.height), 0.1f, 50.0f);
	ubo.proj[1][1] *= -1;
	ubo.particleCount = g_mesh.vertices.size();
	ubo.deltaTime = deltaTime;

	std::memcpy(m_uniformBuffers[imageIndex].second, &ubo, sizeof(ubo));
}

void Vulkan_Renderer::DrawImGui()
{
	auto& io = ImGui::GetIO();
	{
		ImGui::Begin("Voxel FYP");

		ImGui::Checkbox("Run Compute", &m_runCompute);

		if (ImGui::CollapsingHeader("Camera Params"))
		{
			static float dragSpeed = 0.1f;
			ImGui::InputFloat("Drag Speed", &dragSpeed);
			ImGui::DragFloat3("Camera Pos", &m_cameraPos[0], dragSpeed);
			ImGui::DragFloat3("Camera Target", &m_cameraTarget[0], dragSpeed);
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();
	}
}
