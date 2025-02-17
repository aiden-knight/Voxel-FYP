#pragma once
#include <vector>
#include <list>
#include "SpatialHash.h"

struct Voxel;

struct SimulatedVoxel
{
	glm::vec4 position;
	//glm::vec4 force; can't implement yet due to complexity
	glm::vec4 velocity;
};

struct Constraint
{
	SimulatedVoxel* first;
	SimulatedVoxel* second;
	glm::vec4 axis;
};

class Simulator
{
public:
	Simulator(std::vector<Voxel>& voxels);
	~Simulator();

	void Update(float deltaTime);
	static bool TestCollision(SimulatedVoxel& lhs, SimulatedVoxel& rhs);
private:
	SpatialHash m_spatialHash;
	std::vector<Voxel>& m_voxelRef;
	std::vector<SimulatedVoxel> m_voxels;
	std::list<Constraint> m_constraints;
	float m_accumulator;
	float m_voxelHalfExtent;

	void ResetSimulator();
	void UpdateConstraints();
	void Explode(const float range, const float strength);

	static std::pair<float, int> TestAABBs(const SimulatedVoxel& lhs, const SimulatedVoxel& rhs);
};

