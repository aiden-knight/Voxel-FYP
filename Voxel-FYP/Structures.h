#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};