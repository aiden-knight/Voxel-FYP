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

#include "Voxeliser.h"
#include "ObjectLoader.h"
#include "GLFW_Window.h"
#include "Structures.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

#include <random>
#include <iostream>
#include <chrono>

Vulkan_Renderer::Vulkan_Renderer(Vulkan_Wrapper *const owner, DevicePtr device, RenderPassPtr renderPass, RenderPassPtr imGuiRenderPass, SwapChainPtr swapChain,
	PipelinePtr pipeline, CommandPoolPtr graphicsPool, DescriptorSetsPtr descriptorSets) :
	m_owner{owner},
	m_deviceRef{device},
	m_swapChainRef{swapChain},
	m_renderPassRef{renderPass},
	m_imGuiRenderPassRef{imGuiRenderPass},
	m_pipelineRef{pipeline},
	m_descriptorSetsRef{descriptorSets},
	m_commandBuffers{graphicsPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT)},
	m_imguiCommandBuffers{graphicsPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT)},
	m_graphicsPoolRef{graphicsPool}
{
	m_clearColour = { {0.0f, 0.0f, 0.0f, 1.0f} };
	CreateRenderer(device, graphicsPool);
}

Vulkan_Renderer::~Vulkan_Renderer()
{
}

void Vulkan_Renderer::DrawFrame() 
{
	if (m_owner->GetWindow()->resized)
	{
		m_owner->RecreateSwapChain();
		return;
	}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DrawImGui();

	ImGui::Render();

	// graphics pipeline pass
	std::array<vk::Fence, 1> fences{ m_inFlightFence[m_currentFrame] };
	{ auto discard = m_deviceRef->GetHandle().waitForFences(fences, vk::True, UINT64_MAX); }

	std::pair<vk::Result, uint32_t> ret = m_swapChainRef->GetHandle().acquireNextImage(UINT64_MAX, m_imageAvailableSemaphore[m_currentFrame]);
	if (ret.first != vk::Result::eSuccess && ret.first != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}
	
	UpdateUniforms(m_currentFrame);
	UpdateVertexBuffer(m_currentFrame);

	m_deviceRef->GetHandle().resetFences(fences);
	m_commandBuffers[m_currentFrame].reset();
	RecordCommandBuffer(ret.second);
	
	m_imguiCommandBuffers[m_currentFrame].reset();
	RecordImGuiCommandBuffer(ret.second);

	// submit info
	std::vector<vk::Semaphore> waitSemaphores{ m_imageAvailableSemaphore[m_currentFrame] };
	std::vector<vk::PipelineStageFlags> waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };	
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
	if (result == vk::Result::eSuboptimalKHR || m_owner->GetWindow()->resized) {
		m_owner->RecreateSwapChain();
	} else if (result != vk::Result::eSuccess) {
		throw std::runtime_error("failed to present swap chain image");
	}

	// move to next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

	std::array<vk::Buffer, 1> vertexBuffers{ m_vertexBuffers[m_currentFrame].first.GetHandle()};
	std::array<vk::DeviceSize, 1> offsets{ 0 };
	m_commandBuffers[m_currentFrame].bindVertexBuffers(0, vertexBuffers, offsets);
	m_commandBuffers[m_currentFrame].draw(m_voxels.size(), 1, 0, 0);

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

void Vulkan_Renderer::CentreMesh(Mesh& mesh)
{
	glm::vec3 offset = glm::vec3(0.0f);
	float highestDiff = 0;
	for (int i = 0; i < 3; ++i)
	{
		float avg = (mesh.max[i] + mesh.min[i]) / 2;
		float diff = (mesh.max[i] - mesh.min[i]) / 2;

		offset[i] = -avg;

		if (diff > highestDiff)
		{
			highestDiff = diff;
		}
	}

	float modelMult = ImGuiConfig::GetInstance()->modelHalfExtent / highestDiff;
	for (auto& vertex : mesh.vertices)
	{
		vertex.pos += offset;
		vertex.pos *= modelMult;
	}
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

void Vulkan_Renderer::CreateVoxelisationBuffers(DevicePtr device, CommandPoolPtr graphicsPool)
{
	ImGuiConfig* config = ImGuiConfig::GetInstance();

	// clear previous data
	m_vertexBuffers.clear();
	m_voxels.clear();

	// load the mesh and centre it around the world centre for voxelisation
	m_mesh = ObjectLoader::LoadMesh("models\\" + config->modelString + ".obj");
	CentreMesh(m_mesh);

	// half extent of a single voxel and number of voxels in grid in total
	m_voxelHalfExtent = config->modelHalfExtent / config->modelResolution;

	// get the size of the entire voxel grid
	size_t voxelCount = config->modelResolution * config->modelResolution * config->modelResolution;
	vk::DeviceSize bufferSize = sizeof(Voxel) * voxelCount;

	// create buffer to store the voxelisation in
	Vulkan_Buffer voxelisationBuffer = Vulkan_Buffer(device, bufferSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	{
		// data required to voxelise the mesh
		VoxelisationUniform voxelUniform;
		voxelUniform.halfExtent = config->modelHalfExtent;
		voxelUniform.voxelResolution = config->modelResolution;
		voxelUniform.indexCount = m_mesh.indices.size();

		// initialise the voxeliser compute pass then perform it
		Voxeliser voxeliser{ device, graphicsPool, &voxelisationBuffer, m_mesh, voxelUniform };
		voxeliser.Voxelise(device, graphicsPool, config->modelResolution);
	}

	// copy the buffer back to the CPU
	auto before = std::chrono::high_resolution_clock::now();
	float vectorTime;
	{
		Vulkan_Buffer buffer{ device, bufferSize, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
		buffer.CopyFromBuffer(graphicsPool, voxelisationBuffer, bufferSize);
		device->GetHandle().waitIdle();
		Voxel* voxelisation = static_cast<Voxel*>(buffer.MapMemory());

		// fill a vector with the buffer info that has a voxel value
		auto before = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < voxelCount; ++i)
		{
			if (voxelisation->position.w >= 1.0f)
			{
				auto& particle = m_voxels.emplace_back(std::move(*voxelisation));
				particle.position.w = m_voxelHalfExtent;
			}
			++voxelisation;
		}
		auto after = std::chrono::high_resolution_clock::now();
		vectorTime = std::chrono::duration<float, std::chrono::seconds::period>(after - before).count();

		std::cout << "Filling voxelisation vector took: " << vectorTime << " seconds" << std::endl;
	}

	auto after = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(after - before).count();

	std::cout << "Voxelisation took: " << deltaTime << " seconds in total" << std::endl;
	std::cout << "Filling vector was: " << static_cast<int>(100*(vectorTime/deltaTime)) << " percent of the time" << std::endl;


	// create the buffers for each frame 
	bufferSize = m_voxels.size() * sizeof(Voxel);

	Vulkan_Buffer buffer{ device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	buffer.FillBuffer(m_voxels.data());
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		auto& [vertexBuffer, vBufferMemory] = m_vertexBuffers.emplace_back(Vulkan_Buffer(device, bufferSize,
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent), nullptr);

		vBufferMemory = vertexBuffer.MapMemory();
	}
	device->GetHandle().waitIdle();
	config->resetSimulator = true;
}

void Vulkan_Renderer::CreateFrameData(DevicePtr device)
{
	m_imageAvailableSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFence.reserve(MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		// create sync objects
		m_imageAvailableSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_renderFinishedSemaphore.emplace_back(device->GetHandle(), vk::SemaphoreCreateInfo());
		m_inFlightFence.emplace_back(device->GetHandle(), vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));

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

		m_deviceRef->GetHandle().updateDescriptorSets(descriptorWrites, {});
	}
}

void Vulkan_Renderer::CreateRenderer(DevicePtr device, CommandPoolPtr graphicsPool)
{
	CreateVoxelisationBuffers(device, graphicsPool);
	CreateFrameData(device);
}

void Vulkan_Renderer::UpdateUniforms(uint32_t imageIndex)
{
	vk::Extent2D extent = m_swapChainRef->GetImageExtent();
	ImGuiConfig* config = ImGuiConfig::GetInstance();

	UniformBufferObject ubo{};
	ubo.view = glm::lookAt(config->cameraPos, config->cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / static_cast<float>(extent.height), 0.1f, 50.0f);
	ubo.proj[1][1] *= -1;
	ubo.halfExtent = m_voxelHalfExtent;

	std::memcpy(m_uniformBuffers[imageIndex].second, &ubo, sizeof(ubo));
}

void Vulkan_Renderer::UpdateVertexBuffer(uint32_t currentFrame)
{
	std::memcpy(m_vertexBuffers[currentFrame].second, m_voxels.data(), m_voxels.size() * sizeof(Voxel));
}

void Vulkan_Renderer::DrawImGui()
{
	ImGuiConfig* config = ImGuiConfig::GetInstance();
	auto& io = ImGui::GetIO();

	{
		ImGui::Begin("Voxel FYP");

		ImGui::Checkbox("Simulate", &config->simulate);
		ImGui::DragFloat("Time Scale", &config->timeScale, 0.1f);
		ImGui::DragFloat("Breaking Point", &config->breakingPoint, 0.1f);

		if (ImGui::Button("Explode"))
		{
			config->explode = true;
		}
		ImGui::DragFloat("Explosion Force", &config->explosionForce, 0.01f);
		ImGui::DragFloat("Explosion Range", &config->explosionRange, 0.01f, 0.0f, std::numeric_limits<float>::max());

		if (ImGui::CollapsingHeader("Camera Params"))
		{
			static float dragSpeed = 0.1f;
			ImGui::InputFloat("Drag Speed", &dragSpeed);
			ImGui::DragFloat3("Camera Pos", &config->cameraPos[0], dragSpeed);
			ImGui::DragFloat3("Camera Target", &config->cameraTarget[0], dragSpeed);
		}

		if (ImGui::CollapsingHeader("Model Params"))
		{
			ImGui::InputText("Model", &config->modelString);
			ImGui::DragFloat("Target Half Extent", &config->modelHalfExtent, 0.1f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragInt("Resolution", &config->modelResolution, 1.0f, 0, std::numeric_limits<int>::max());

			if (ImGui::Button("Load New Model"))
			{
				m_deviceRef->GetHandle().waitIdle();
				CreateVoxelisationBuffers(m_deviceRef, m_graphicsPoolRef);
			}
		}

		if (ImGui::CollapsingHeader("Profiling Params"))
		{
			ImGui::Separator();
			ImGui::Text("On Start:");
			static bool enableSim = false;
			ImGui::Checkbox("Enable simulation", &enableSim);

			static bool reloadModel = false;
			ImGui::Checkbox("Reload Model", &reloadModel);

			ImGui::Separator();
			ImGui::Text("Other:");
			static int dataCount = 1000;
			ImGui::DragInt("Data Count", &dataCount, 1, 0, std::numeric_limits<int>::max());

			ImGui::Separator();
			if (config->profile)
			{
				ImGui::Text("Currently profiling: true");
			}
			else
			{
				ImGui::Text("Currently profiling: false");
			}

			if (ImGui::Button("Begin Profiling") && !config->profile)
			{
				if (reloadModel)
				{
					m_deviceRef->GetHandle().waitIdle();
					CreateVoxelisationBuffers(m_deviceRef, m_graphicsPoolRef);
				}

				if (enableSim)
					config->simulate = true;

				config->profileInfo = ProfileInfo(dataCount, m_voxels.size(), config->modelResolution, config->simulate, config->modelString);
				config->profile = true;
			}
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();
	}
}