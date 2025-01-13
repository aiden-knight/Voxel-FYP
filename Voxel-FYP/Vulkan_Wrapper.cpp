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
#include "GLFW_Window.h"

Vulkan_Wrapper::Vulkan_Wrapper(GLFW_Window* window, bool validationEnabled) :
	m_validationLayersEnabled(validationEnabled),
	m_ctx{}
{
	m_window = window;
	m_instance.reset(new Vulkan_Instance(this));
	m_debugMessenger.reset(new Vulkan_Debugger(m_instance));
	m_surface.reset(new Vulkan_Surface(m_instance, m_window));
	m_device.reset(new Vulkan_Device(m_instance, m_surface));

	int width, height;
	m_window->GetFramebufferSize(&width, &height);
	m_swapChain.reset(new Vulkan_SwapChain(m_device, m_surface, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, nullptr));

	m_renderPass.reset(new Vulkan_RenderPass(m_device, m_swapChain->GetImageFormat()));
	m_pipeline.reset(new Vulkan_Pipeline(m_device, m_renderPass));

	m_swapChain->CreateFramebuffers(m_device, m_renderPass);
	m_graphicsPool.reset(new Vulkan_CommandPool(m_device, GRAPHICS));
	m_renderer.reset(new Vulkan_Renderer(this, m_device, m_renderPass, m_swapChain, m_pipeline, m_graphicsPool));
}

Vulkan_Wrapper::~Vulkan_Wrapper()
{
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

	m_swapChain->CreateFramebuffers(m_device, m_renderPass);
}
