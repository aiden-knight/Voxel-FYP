#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <list>

struct VoxelNode;

class SpatialHash
{
public:
	void Clear();
	void Insert(VoxelNode* node);
	void Update();
	void TestCollisions();

	glm::ivec3 GetPos(VoxelNode* node);
private:
	std::unordered_map<glm::ivec3, std::list<VoxelNode*>> m_hashMap;

};