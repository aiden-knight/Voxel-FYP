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

void SpatialHash::Insert(Voxel* node)
{
	glm::ivec3 pos{};
	for (int i = 0; i < 3; ++i)
	{
		pos[i] = static_cast<int>(floor(node->position[i] / (2 * node->position.w)));
	}
	
	m_hashMap[pos].push_back(node);
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
