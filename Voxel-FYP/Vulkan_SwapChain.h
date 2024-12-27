#pragma once
#define WIN32
#include <vulkan/vulkan_raii.hpp>
#include <vector>

#include "Vulkan_FWD.h"

class Vulkan_SwapChain
{
public:
	Vulkan_SwapChain(DevicePtr device, SurfacePtr surface, vk::Extent2D surfaceExtent, const Vulkan_SwapChain* oldSwapchain);

    const vk::raii::SwapchainKHR& GetHandle() const { return m_swapChain; }

    const vk::Format GetImageFormat() const { return m_imageFormat; }
    const vk::Extent2D GetImageExtent() const { return m_imageExtent; }

    const vk::raii::Framebuffer& GetFramebuffer(uint32_t index) const { return m_frameBuffers[index]; }
    void CreateFramebuffers(DevicePtr device, RenderPassPtr renderPass);
private:
    const vk::Extent2D m_imageExtent;
    const vk::Format m_imageFormat;
	vk::raii::SwapchainKHR m_swapChain;

    std::vector<vk::Image> m_images;
    std::vector<vk::raii::ImageView> m_imageViews;
    std::vector<vk::raii::Framebuffer> m_frameBuffers;


    vk::SwapchainCreateInfoKHR GetCreateInfo(DevicePtr device, SurfacePtr surface, const Vulkan_SwapChain* oldSwapchain) const;

    /// <summary>
    /// Chooses the best image format for the swapchain out of those available
    /// </summary>
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;

    /// <summary>
    /// Chooses the best present mode for the swapchain out of those available
    /// </summary>
    vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;

    /// <summary>
    /// Calculates the swapchain extent that best fits framebuffer but still abides by capabilities
    /// </summary>
    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D windowExtent) const;
};

