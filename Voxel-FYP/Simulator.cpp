#include "Simulator.h"
#include "ImGuiConfig.h"
#include "Structures.h"
#include <cmath>

Simulator::Simulator(std::vector<Voxel>& particles) :
	m_particleRef{particles}
{
	m_accumulator = 0;
}

Simulator::~Simulator()
{
}

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
		m_spatialHash.Clear();
		config->resetSimulator = false;
	}

	if (!config->simulate) return;

	for (size_t i = 0; i < m_particleRef.size(); ++i)
	{
		auto& particle = m_particleRef[i];

		particle.position += particle.velocity * deltaTime * config->timeScale;
		particle.velocity.y += (-1 * deltaTime * config->timeScale);

		if (particle.position.y <= -2.0f)
		{
			particle.velocity.y *= -1;
			particle.position.y = -1.9f;
		}
		
		spatialHash.Insert(&m_particleRef[i]);
	}

	spatialHash.TestCollisions();
}

bool Simulator::TestCollision(Voxel& lhs, Voxel& rhs)
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

	lhs.position += collisionNormal * penDepth * 0.5f;
	rhs.position -= collisionNormal * penDepth * 0.5f;

	float restitution = 1.0f;
	float vj = -(1.0f + restitution) * dotProd;
	float energy = vj / 2.0f;

	lhs.velocity += energy * collisionNormal;
	rhs.velocity -= energy * collisionNormal;
	return true;
}

std::pair<float, int> Simulator::TestAABBs(const Voxel& lhs, const Voxel& rhs)
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
