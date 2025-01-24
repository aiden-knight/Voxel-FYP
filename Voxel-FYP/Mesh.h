#pragma once
#include "Structures.h"
#include "Vulkan_FWD.h"

struct Mesh
{
	glm::vec3 min;
	glm::vec3 max;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};