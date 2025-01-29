#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define WIN32
#include <vulkan/vulkan_raii.hpp>

// Vertex format for use in vertex shader
struct Vertex 
{
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 normal;

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

struct Particle
{
	glm::vec4 position;
	glm::vec4 velocity;
	glm::vec4 colour;

	static std::array<vk::VertexInputBindingDescription, 1> GetBindingDescription() {
		vk::VertexInputBindingDescription bindingDesc{
			0, sizeof(Particle), vk::VertexInputRate::eVertex
		};

		return { bindingDesc };
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() {
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{ {
				//	Location	Binding		Format aka how many bytes			Offset from vertex start
				{	0,			0,			vk::Format::eR32G32B32A32Sfloat,	offsetof(Particle, position) },
				{	1,			0,			vk::Format::eR32G32B32A32Sfloat,	offsetof(Particle, colour) }
			} };

		return attributeDescriptions;
	}
};

// Used for number of framebuffers etc
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Uniform buffer object for passing data to shader every frame (ideally once)
struct UniformBufferObject {
	glm::mat4 proj;
	glm::mat4 view;
	unsigned int particleCount;
	float halfExtent;
	float velocityMult;
	float deltaTime;
};

struct VoxelisationUniform {
	int voxelResolution;
	float halfExtent;
	unsigned int indexCount;
};