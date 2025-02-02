#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>

struct Voxel;

class SpatialHash
{
public:
	void Clear();
	void Insert(Voxel* node);
	void TestCollisions();

private:
	std::unordered_map<glm::ivec3, std::vector<Voxel*>> m_hashMap;
};