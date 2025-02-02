#include "Simulator.h"
#include "ImGuiConfig.h"
#include "Structures.h"
#include <cmath>
#include <random>

consteval std::array<glm::ivec3, 26> CreateOffsets()
{
	std::array<glm::ivec3, 26> offsets;
	int index = 0;
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			for (int z = -1; z <= 1; ++z)
			{
				if (x == 0 && y == 0 && z == 0)
					continue;

				offsets[index++] = glm::ivec3(x, y, z);
			}
		}
	}
	return offsets;
}

constexpr std::array<glm::ivec3, 26> offsets = CreateOffsets();

Simulator::Simulator(std::vector<Voxel>& particles) :
	m_particleRef{particles}
{
	m_accumulator = 0;
}

Simulator::~Simulator()
{
}

void Simulator::ResetSimulator()
{
	m_voxels.clear();
	m_voxels.reserve(m_particleRef.size());
	std::unordered_map<glm::ivec3, VoxelNode*> voxelNodeMap;

	m_voxelHalfExtent = m_particleRef.front().position.w;

	for (size_t i = 0; i < m_particleRef.size(); ++i)
	{
		auto& voxelNode = m_voxels.emplace_back(VoxelNode(&m_particleRef[i], {}, false));
		voxelNode.constraints.resize(26, nullptr);
	}

	for (size_t i = 0; i < m_voxels.size(); ++i)
	{
		auto& voxelNode = m_voxels[i];
		glm::ivec3 pos = m_spatialHash.GetPos(&voxelNode);
		voxelNodeMap[pos] = &voxelNode;

		for (size_t j = 0; j < offsets.size(); ++j)
		{
			glm::ivec3 offPos = pos + offsets[j];
			if (voxelNodeMap.contains(offPos))
			{
				VoxelNode* otherNode = voxelNodeMap[offPos];
				voxelNode.constraints[j] = otherNode;
				otherNode->constraints[25 - j] = &voxelNode;
			}
		}	
	}

	GetAllCounts();
}

void Simulator::UpdateConstraints(VoxelNode& voxelNode)
{
	auto& particle = *voxelNode.voxel;

	for (size_t j = 0; j < offsets.size(); ++j)
	{
		if (voxelNode.constraints[j] == nullptr || voxelNode.constraints[j]->updated)  continue;

		glm::vec4 toAdd{ static_cast<glm::vec3>(offsets[j]) * 2.0f * m_voxelHalfExtent , 0.0f };
		voxelNode.constraints[j]->voxel->position = particle.position + toAdd;
		voxelNode.constraints[j]->voxel->velocity = particle.velocity;
		voxelNode.constraints[j]->updated = true;
		UpdateConstraints(*voxelNode.constraints[j]);
	}
}

size_t Simulator::GetCount(VoxelNode& voxelNode, size_t current)
{
	for (size_t j = 0; j < offsets.size(); ++j)
	{
		if (voxelNode.constraints[j] == nullptr || voxelNode.constraints[j]->updated)  continue;

		voxelNode.constraints[j]->updated = true;
		current = GetCount(*voxelNode.constraints[j], current + 1);
	}
	return current;
}

void Simulator::PropegateCount(VoxelNode& voxelNode)
{
	for (size_t j = 0; j < offsets.size(); ++j)
	{
		if (voxelNode.constraints[j] == nullptr || voxelNode.constraints[j]->updated)  continue;

		voxelNode.constraints[j]->voxelCount = voxelNode.voxelCount;
		voxelNode.constraints[j]->updated = true;
		PropegateCount(*voxelNode.constraints[j]);
	}
}

void Simulator::Explode(float strength)
{
	static std::random_device rand{};
	static std::mt19937_64 engine{ rand() };
	std::uniform_int_distribution<int> distribution{ 0, static_cast<int>(m_voxels.size() - 1)};

	int index = distribution(engine);

	for (auto& voxelNode : m_voxels)
		voxelNode.updated = false;

	VoxelNode& source = m_voxels[index];
	source.updated = true;

	PropegateExplosion(source.voxel->position, source, strength);
}

void Simulator::PropegateExplosion(glm::vec4 source, VoxelNode& voxelNode, float strength)
{
	for (size_t j = 0; j < offsets.size(); ++j)
	{
		if (voxelNode.constraints[j] == nullptr)  continue;

		VoxelNode& other = *voxelNode.constraints[j];
		glm::vec4 diff = other.voxel->position - source;
		float distance = glm::length(diff) * 2;
		if (strength > distance)
		{
			voxelNode.constraints[j] = nullptr;
			other.constraints[25 - j] = nullptr;
			if (!other.updated)
			{
				other.updated = true;
				PropegateExplosion(source, other, strength);
			}
		}

		glm::vec4 dir = diff / distance;
		other.voxel->velocity += dir * (((1/distance) * strength)/5);
	}
}

constexpr float wallPos = 8.0f;

void Simulator::Update(float deltaTime)
{
	m_accumulator += deltaTime;

	if (m_accumulator > 0.016f)
		m_accumulator -= 0.016f;
	else
		return;

	deltaTime = 0.016f;
	
	ImGuiConfig* config = ImGuiConfig::GetInstance();
	if (config->resetSimulator)
	{
		config->resetSimulator = false;
		ResetSimulator();
	}

	if (config->explode)
	{
		config->explode = false;
		Explode(config->explosionStrength);
	}

	if (!config->simulate) return;

	m_spatialHash.Clear();

	for (auto& voxelNode : m_voxels)
		voxelNode.updated = false;

	for (auto& voxelNode : m_voxels)
	{
		if (!voxelNode.updated)
		{
			auto& particle = *voxelNode.voxel;

			particle.position += particle.velocity * deltaTime * config->timeScale;
			particle.velocity.y += (-1 * deltaTime * config->timeScale);

			float deprec = 0.3f;

			if (particle.position.y <= -2.0f)
			{
				particle.velocity.y = abs(particle.velocity.y) * deprec;
				particle.position.y = -1.9f;
			}

			if (particle.position.x <= -wallPos)
			{
				particle.velocity.x = abs(particle.velocity.x) * deprec;
				particle.position.x = -wallPos;
			}
			if (particle.position.x >= wallPos)
			{
				particle.velocity.x = abs(particle.velocity.x) * -deprec;
				particle.position.x = wallPos;
			}

			if (particle.position.z <= -wallPos)
			{
				particle.velocity.z = abs(particle.velocity.z) * deprec;
				particle.position.z = -wallPos;
			}
			if (particle.position.z >= wallPos)
			{
				particle.velocity.z = abs(particle.velocity.z) * -deprec;
				particle.position.z = wallPos;
			}

			UpdateConstraints(voxelNode);
		}
		
		m_spatialHash.Insert(&voxelNode);
	}

	m_spatialHash.TestCollisions();
}

bool Simulator::TestCollision(VoxelNode& lhs, VoxelNode& rhs)
{
	auto [penDepth, axis] = TestAABBs(lhs, rhs);
	if (axis == -1) return false;

	glm::vec4 relativeVelocity = lhs.voxel->velocity - rhs.voxel->velocity;

	int mult = penDepth < 0 ? -1 : 1;
	penDepth *= mult;
	glm::vec4 collisionNormal = glm::vec4(0.0f);
	collisionNormal[axis] = mult;

	float dotProd = glm::dot(collisionNormal, relativeVelocity);
	if (dotProd <= 0.0f) return false;

	float invMassLhs = 1.0f / lhs.voxelCount;
	float invMassRhs = 1.0f / rhs.voxelCount;
	float invMassSum = invMassLhs + invMassRhs;

	lhs.voxel->position += collisionNormal * penDepth * (invMassLhs / invMassSum);
	rhs.voxel->position -= collisionNormal * penDepth * (invMassRhs / invMassSum);

	float restitution = 0.5f;
	float vj = -(1.0f + restitution) * dotProd;
	float energy = vj / 2.0f;

	lhs.voxel->velocity += invMassLhs * energy * collisionNormal;
	rhs.voxel->velocity -= invMassRhs * energy * collisionNormal;
	return true;
}

void Simulator::GetAllCounts()
{
	for (auto& voxelNode : m_voxels)
		voxelNode.updated = false;

	for (auto& voxelNode : m_voxels)
	{
		if (voxelNode.updated) continue;

		voxelNode.updated = true;
		voxelNode.voxelCount = GetCount(voxelNode, 1);
	}

	for (auto& voxelNode : m_voxels)
		voxelNode.updated = false;

	for (auto& voxelNode : m_voxels)
	{
		if (voxelNode.updated) continue;

		PropegateCount(voxelNode);
	}
}

std::pair<float, int> Simulator::TestAABBs(const VoxelNode& lhs, const VoxelNode& rhs)
{
	float voxelHalfExtent = lhs.voxel->position.w;
	std::pair<float, int> result = { std::numeric_limits<float>::max(), -1 };

	int multi = 1;
	for (int i = 0; i < 3; i++)
	{
		float lhsMin = lhs.voxel->position[i] - voxelHalfExtent;
		float lhsMax = lhs.voxel->position[i] + voxelHalfExtent;
		float rhsMin = rhs.voxel->position[i] - voxelHalfExtent;
		float rhsMax = rhs.voxel->position[i] + voxelHalfExtent;

		if (rhsMax - lhsMin < result.first)
		{
			result.second = i;
			result.first = rhsMax - lhsMin;
			multi = -1;
		}
		else if(lhsMax - rhsMin < result.first)
		{
			result.second = i;
			result.first = lhsMax - rhsMin;
			multi = 1;
		}

		if (lhsMin >= rhsMax) return { -1, -1 };
		if (lhsMax <= rhsMin) return { -1, -1 };
	}

	result.first *= multi;

	return result;
}
