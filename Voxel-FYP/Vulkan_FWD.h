#pragma once
#include <memory>

class Vulkan_CommandPool;
class Vulkan_Debugger;
class Vulkan_Device;
class Vulkan_Instance;
class Vulkan_Pipeline;
class Vulkan_Renderer;
class Vulkan_RenderPass;
class Vulkan_Surface;
class Vulkan_SwapChain;
class Vulkan_Buffer;

using CommandPoolPtr = const std::unique_ptr<Vulkan_CommandPool>&;
using DebuggerPtr = const std::unique_ptr<Vulkan_Debugger>&;
using DevicePtr = const std::unique_ptr<Vulkan_Device>&;
using InstancePtr = const std::unique_ptr<Vulkan_Instance>&;
using PipelinePtr = const std::unique_ptr<Vulkan_Pipeline>&;
using RendererPtr = const std::unique_ptr<Vulkan_Renderer>&;
using RenderPassPtr = const std::unique_ptr<Vulkan_RenderPass>&;
using SurfacePtr = const std::unique_ptr<Vulkan_Surface>&;
using SwapChainPtr = const std::unique_ptr<Vulkan_SwapChain>&;
using BufferPtr = const std::unique_ptr<Vulkan_Buffer>&;