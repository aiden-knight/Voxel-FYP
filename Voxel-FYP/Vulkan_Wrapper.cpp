#include "Vulkan_Wrapper.h"
#include "Vulkan_Instance.h"
#include "Vulkan_Debugger.h"
#include "Vulkan_Surface.h"
#include "Vulkan_Device.h"
#include "Vulkan_SwapChain.h"
#include "Vulkan_RenderPass.h"
#include "Vulkan_Pipeline.h"
#include "Vulkan_CommandPool.h"
#include "Vulkan_Renderer.h"
#include "Vulkan_DescriptorSets.h"
#include "Vulkan_Image.h"
#include "Structures.h"
#include "GLFW_Window.h"
#include "Voxeliser.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

Vulkan_Wrapper::Vulkan_Wrapper(GLFW_Window* window, bool validationEnabled) :
	m_validationLayersEnabled(validationEnabled),
	m_ctx{}
{
	m_window = window;
	m_instance.reset(new Vulkan_Instance(this));
	m_debugMessenger.reset(new Vulkan_Debugger(m_instance));
	m_surface.reset(new Vulkan_Surface(m_instance, m_window));
	m_device.reset(new Vulkan_Device(m_instance, m_surface));

	// graphics pipeline objects
	InitialiseGraphics();

	// command pool for rendering commands
	m_graphicsPool.reset(new Vulkan_CommandPool(m_device, GRAPHICS));

	// imgui initialisation
	InitialiseImGUI();

	// renderer
	m_renderer.reset(new Vulkan_Renderer(this, m_device, m_renderPass, m_imGuiRenderPass, m_swapChain, m_pipeline, m_graphicsPool, m_descriptorSets));
}

Vulkan_Wrapper::~Vulkan_Wrapper()
{
	m_device->GetHandle().waitIdle();

	ImGui_ImplVulkan_Shutdown();
	m_window->ShutdownImGui();
	ImGui::DestroyContext();
	
	(*m_device->GetHandle()).destroyDescriptorPool(m_imGuiDescriptorPool);
}

std::vector<const char*> Vulkan_Wrapper::GetValidationLayers() const
{
	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	return validationLayers;
}

void Vulkan_Wrapper::RecreateSwapChain() 
{
	int width = 0, height = 0;
	m_window->GetFramebufferSize(&width, &height);
	while (width == 0 || height == 0) {
		m_window->GetFramebufferSize(&width, &height);
		m_window->WaitEvents();
	}

	m_device->GetHandle().waitIdle();
	m_device->ResetSwapChainSupportDetails(m_surface);

	m_swapChain.reset(new Vulkan_SwapChain(m_device, m_surface, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }, m_swapChain.get()));
	m_depthImage.reset(new Vulkan_Image(m_device, vk::Extent3D(m_swapChain->GetImageExtent(), 1), m_device->FindDepthFormat(),
		vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth));

	m_swapChain->CreateFramebuffers(m_device, m_renderPass, m_depthImage);
	m_swapChain->CreateImGuiFramebuffers(m_device, m_imGuiRenderPass);
	m_window->resized = false;
}

void Vulkan_Wrapper::InitialiseGraphics()
{
	int width, height;
	m_window->GetFramebufferSize(&width, &height);
	m_swapChain.reset(new Vulkan_SwapChain(m_device, m_surface, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }, nullptr));

	std::vector<vk::DescriptorSetLayoutBinding> graphicsDescriptors{ {
		{
			0, // binding
			vk::DescriptorType::eUniformBuffer,
			1, // descripter count
			vk::ShaderStageFlagBits::eGeometry
		}
	} };
	m_descriptorSets.reset(new Vulkan_DescriptorSets(m_device, graphicsDescriptors, MAX_FRAMES_IN_FLIGHT));
	m_depthImage.reset(new Vulkan_Image(m_device, vk::Extent3D(m_swapChain->GetImageExtent(), 1), m_device->FindDepthFormat(),
		vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth));
	m_renderPass.reset(new Vulkan_RenderPass(m_device, m_swapChain->GetImageFormat()));
	m_pipeline.reset(new Vulkan_Pipeline(m_device, m_descriptorSets, m_renderPass));

	m_swapChain->CreateFramebuffers(m_device, m_renderPass, m_depthImage);
}

void Vulkan_Wrapper::InitialiseImGUI()
{
	std::vector<vk::DescriptorPoolSize> poolSizes = { {{
		vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT
	}} };

	vk::DescriptorPoolCreateInfo poolInfo = {
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		MAX_FRAMES_IN_FLIGHT,
		poolSizes
	};
	m_imGuiDescriptorPool = (*m_device->GetHandle()).createDescriptorPool(poolInfo);
	m_imGuiRenderPass.reset(new Vulkan_RenderPass(m_device, m_swapChain->GetImageFormat(), true));
	m_swapChain->CreateImGuiFramebuffers(m_device, m_imGuiRenderPass);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	m_window->InitImGui();

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = *m_instance->GetHandle();
	m_device->FillImGui_InitInfo(initInfo);

	initInfo.PipelineCache = VK_NULL_HANDLE; // optional
	initInfo.DescriptorPool = m_imGuiDescriptorPool;
	initInfo.RenderPass = *m_imGuiRenderPass->GetHandle();
	initInfo.Subpass = 0; // optional
	initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
	initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr; // optional
	initInfo.CheckVkResultFn = m_debugMessenger->CheckVkResult;
	ImGui_ImplVulkan_Init(&initInfo);
}
