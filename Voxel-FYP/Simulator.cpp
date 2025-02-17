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

Simulator::Simulator(std::vector<Voxel>& voxels) :
	m_voxelRef{ voxels }
{
	m_accumulator = 0;
}

Simulator::~Simulator()
{
}

void Simulator::ResetSimulator()
{
	m_constraints.clear();
	m_voxels.clear();
	std::unordered_map<glm::ivec3, SimulatedVoxel*> voxelMap;

	m_voxelHalfExtent = m_voxelRef.front().position.w;
	m_voxels.reserve(m_voxelRef.size());

	for (const auto& voxelRef : m_voxelRef)
	{
		m_voxels.emplace_back(voxelRef.position);
	}

	for (auto& voxel : m_voxels)
	{
		glm::ivec3 pos = m_spatialHash.GetPos(&voxel);
		voxelMap[pos] = &voxel;

		for (size_t j = 0; j < offsets.size(); ++j)
		{
			glm::ivec3 offPos = pos + offsets[j];
			if (voxelMap.contains(offPos))
			{
				m_constraints.emplace_back(&voxel, voxelMap[offPos]);
			}
		}
	}
}

void Simulator::UpdateConstraints()
{
	
}

void Simulator::Explode(float strength)
{
	static std::random_device rand{};
	static std::mt19937_64 engine{ rand() };
	std::uniform_int_distribution<int> distribution{ 0, static_cast<int>(m_voxelRef.size() - 1)};

	int index = distribution(engine);


	Voxel& source = m_voxelRef[index];

	PropegateExplosion(source.position, strength);
}

void Simulator::PropegateExplosion(glm::vec4 source, float strength)
{
}

constexpr float wallPos = 8.0f;

void Simulator::Update(float deltaTime)
{
	m_accumulator += deltaTime;

	if (m_accumulator > 0.016f)
		m_accumulator -= 0.016f;
	else
		return;

	ImGuiConfig* config = ImGuiConfig::GetInstance();
	if (!config->simulate) return;

	deltaTime = 0.016f;
	
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

	m_spatialHash.Clear();

	for (auto& voxel : m_voxels)
	{
		voxel.position += voxel.velocity * deltaTime * config->timeScale;
		voxel.velocity.y += (-1 * deltaTime * config->timeScale);

		float deprec = 0.3f;

		if (voxel.position.y <= -2.0f)
		{
			voxel.velocity.y = abs(voxel.velocity.y) * deprec;
			voxel.position.y = -1.9f;
		}

		if (voxel.position.x <= -wallPos)
		{
			voxel.velocity.x = abs(voxel.velocity.x) * deprec;
			voxel.position.x = -wallPos;
		}
		if (voxel.position.x >= wallPos)
		{
			voxel.velocity.x = abs(voxel.velocity.x) * -deprec;
			voxel.position.x = wallPos;
		}

		if (voxel.position.z <= -wallPos)
		{
			voxel.velocity.z = abs(voxel.velocity.z) * deprec;
			voxel.position.z = -wallPos;
		}
		if (voxel.position.z >= wallPos)
		{
			voxel.velocity.z = abs(voxel.velocity.z) * -deprec;
			voxel.position.z = wallPos;
		}
		
		m_spatialHash.Insert(&voxel);
	}

	m_spatialHash.TestCollisions();

	for (size_t index = 0; index < m_voxelRef.size(); ++index)
	{
		m_voxelRef[index].position = m_voxels[index].position;
	}
}

bool Simulator::TestCollision(SimulatedVoxel& lhs, SimulatedVoxel& rhs)
{
	auto [penDepth, axis] = TestAABBs(lhs, rhs);
	if (axis == -1) return false;

	glm::vec4 relativeVelocity = lhs.velocity - rhs.velocity;

	int mult = penDepth < 0 ? -1 : 1;
	penDepth *= mult;
	glm::vec4 collisionNormal = glm::vec4(0.0f);
	collisionNormal[axis] = mult;

	float dotProd = glm::dot(collisionNormal, relativeVelocity);
	if (dotProd <= 0.0f) return false;

	float invMassLhs = 1.0f;
	float invMassRhs = 1.0f;
	float invMassSum = invMassLhs + invMassRhs;

	lhs.position += collisionNormal * penDepth * (invMassLhs / invMassSum);
	rhs.position -= collisionNormal * penDepth * (invMassRhs / invMassSum);

	float restitution = 0.5f;
	float vj = -(1.0f + restitution) * dotProd;
	float energy = vj / invMassSum;

	lhs.velocity += invMassLhs * energy * collisionNormal;
	rhs.velocity -= invMassRhs * energy * collisionNormal;
	return true;
}

std::pair<float, int> Simulator::TestAABBs(const SimulatedVoxel& lhs, const SimulatedVoxel& rhs)
{
	float voxelHalfExtent = lhs.position.w;
	std::pair<float, int> result = { std::numeric_limits<float>::max(), -1 };

	int multi = 1;
	for (int i = 0; i < 3; i++)
	{
		float lhsMin = lhs.position[i] - voxelHalfExtent;
		float lhsMax = lhs.position[i] + voxelHalfExtent;
		float rhsMin = rhs.position[i] - voxelHalfExtent;
		float rhsMax = rhs.position[i] + voxelHalfExtent;

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
