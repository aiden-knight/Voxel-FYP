#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define WIN32
#include <vulkan/vulkan_raii.hpp>

struct Vertex 
{
	glm::vec2 pos;
	glm::vec3 colour;

	static std::array<vk::VertexInputBindingDescription, 1> GetBindingDescription() {
		vk::VertexInputBindingDescription bindingDesc{
			0, sizeof(Vertex), vk::VertexInputRate::eVertex
		};

		return { bindingDesc };
	}

	static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{ {
			{ 0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos) },
			{ 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, colour) }
		} };

		return attributeDescriptions;
	}
};