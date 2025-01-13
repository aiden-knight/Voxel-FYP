#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define WIN32
#include <vulkan/vulkan_raii.hpp>

struct Vertex 
{
	glm::uint32 pos;

	static std::array<vk::VertexInputBindingDescription, 1> GetBindingDescription() {
		vk::VertexInputBindingDescription bindingDesc{
			0, sizeof(Vertex), vk::VertexInputRate::eVertex
		};

		return { bindingDesc };
	}

	static std::array<vk::VertexInputAttributeDescription, 1> GetAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 1> attributeDescriptions{ {
			{ 0, 0, vk::Format::eR32Uint, offsetof(Vertex, pos) }
		} };

		return attributeDescriptions;
	}
};