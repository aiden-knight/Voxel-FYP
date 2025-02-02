#pragma once
#include <vector>
#include "SpatialHash.h"

struct Voxel;

struct VoxelNode
{
	Voxel* voxel;
	std::vector<VoxelNode*> constraints;
	bool updated;
	unsigned int voxelCount = 0;
};

class Simulator
{
public:
	Simulator(std::vector<Voxel>& particles);
	~Simulator();

	void Update(float deltaTime);
	static bool TestCollision(VoxelNode& lhs, VoxelNode& rhs);
private:
	SpatialHash m_spatialHash;
	std::vector<Voxel>& m_particleRef;
	std::vector<VoxelNode> m_voxels;
	float m_accumulator;
	float m_voxelHalfExtent;

	void ResetSimulator();
	void UpdateConstraints(VoxelNode& voxelNode);
	size_t GetCount(VoxelNode& voxelNode, size_t current);
	void PropegateCount(VoxelNode& voxelNode);
	void Explode(float strength);
	void PropegateExplosion(glm::vec4 source, VoxelNode& voxelNode, float strength);

	void GetAllCounts();
	static std::pair<float, int> TestAABBs(const VoxelNode& lhs, const VoxelNode& rhs);
};

