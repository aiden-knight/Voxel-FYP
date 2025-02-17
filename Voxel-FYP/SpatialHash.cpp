#include "SpatialHash.h"
#include "Structures.h"
#include "Simulator.h"
#include <cmath>

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

void SpatialHash::Clear()
{
	m_hashMap.clear();
}

void SpatialHash::Insert(SimulatedVoxel* node)
{
	m_hashMap[GetPos(node)].push_back(node);
}

struct ListUpdate
{
	std::list<SimulatedVoxel*>::iterator it;
	std::list<SimulatedVoxel*>* old;
	glm::ivec3 newPosition;
};

void SpatialHash::Update()
{
	for (auto& [pos, vec] : m_hashMap)
	{
		for (auto it = vec.begin(); it != vec.end();)
		{
			auto jt = it;
			++it;

			glm::ivec3 currPos = GetPos(*jt);
			if (currPos != pos)
			{
				m_hashMap[currPos].splice(m_hashMap[currPos].begin(), vec, jt);
			}
		}
	}
}

void SpatialHash::TestCollisions()
{
	for (auto& [pos, vec] : m_hashMap)
	{
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			auto jt = it;
			for (++jt; jt != vec.end(); ++jt)
			{
				Simulator::TestCollision(**it, **jt);
			}

			for (const auto& offset : offsets)
			{
				auto& other = m_hashMap[pos + offset];
				for (auto jt = other.begin(); jt != other.end(); ++jt)
				{
					Simulator::TestCollision(**it, **jt);
				}
			}
		}
	}
}

glm::ivec3 SpatialHash::GetPos(const SimulatedVoxel* voxel) const
{
	glm::ivec3 pos{};
	for (int i = 0; i < 3; ++i)
	{
		pos[i] = static_cast<int>(floor(voxel->position[i] / (2 * voxel->position.w)));
	}
	return pos;
}