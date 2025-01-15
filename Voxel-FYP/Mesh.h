#pragma once
#include "Structures.h"
#include "Vulkan_FWD.h"

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};