#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define WIN32
#include <vulkan/vulkan_raii.hpp>

// Vertex format for use in vertex shader
struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 normal;

	static std::array<vk::VertexInputBindingDescription, 1> GetBindingDescription() {
		vk::VertexInputBindingDescription bindingDesc{
			0, sizeof(Vertex), vk::VertexInputRate::eVertex
		};

		return { bindingDesc };
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() {
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{ {
			//	Layout	Binding		Format aka how many bytes		Offset from vertex start
			{	0,		0,			vk::Format::eR32G32B32Sfloat,	offsetof(Vertex, pos) },
			{	1,		0,			vk::Format::eR32G32B32Sfloat,	offsetof(Vertex, normal) }
		} };

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && normal == other.normal;
	}
};

// Used for number of framebuffers etc
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Uniform buffer object for passing data to shader every frame (ideally once)
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};