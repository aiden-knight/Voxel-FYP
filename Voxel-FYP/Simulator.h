#pragma once
#include <vector>
#include "SpatialHash.h"

struct VoxelNode;
struct Voxel;

class Simulator
{
public:
	Simulator(std::vector<Voxel>& particles);
	~Simulator();

	void Update(float deltaTime);
	static bool TestCollision(Voxel& lhs, Voxel& rhs);
private:
	SpatialHash m_spatialHash;
	std::vector<Voxel>& m_particleRef;
	float m_accumulator;

	static std::pair<float, int> TestAABBs(const Voxel& lhs, const Voxel& rhs);
};

