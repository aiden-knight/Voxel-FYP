#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <list>

struct SimulatedVoxel;

class SpatialHash
{
public:
	void Clear();
	void Insert(SimulatedVoxel* voxel);
	void Update();
	void TestCollisions();

	glm::ivec3 GetPos(const SimulatedVoxel* voxel) const;
private:
	std::unordered_map<glm::ivec3, std::list<SimulatedVoxel*>> m_hashMap;

};