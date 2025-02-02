#pragma once
#include <vector>

struct VoxelNode;
struct Voxel;

class Simulator
{
public:
	Simulator(std::vector<VoxelNode>& particles);
	~Simulator();

	void Update(float deltaTime);
	static bool TestCollision(Voxel& lhs, Voxel& rhs);
private:
	std::vector<VoxelNode>& m_particleRef;
	float m_accumulator;

	static std::pair<float, int> TestAABBs(const Voxel& lhs, const Voxel& rhs);
};

