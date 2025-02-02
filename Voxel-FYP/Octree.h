#pragma once
#include <glm/glm.hpp>

#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>

struct VoxelNode;

using Vec3 = glm::vec3;
using ColliderObject = VoxelNode;

class Octree
{
public:
	struct Octant
	{
		// don't need to store extent as long as every object is in root node
		const Vec3 centre;
		std::array<Octant*, 8> children;

		Octant(Vec3 centre, Octant* parent);
		void AddToList(ColliderObject* pObj);
		void TestCollisions();
		void ClearList();

	private:
		ColliderObject* pObjects;
		Octant* pParent;
		std::mutex listMutex;
	};


	Octree(const Vec3 position, const Vec3 extent, const unsigned int maxDepth);
	~Octree();

	void Insert(ColliderObject* pObj);
	void TestCollisions();
	void ClearLists();

private:
	std::vector<std::thread> threads;
	std::queue<Octant*> octantQueue;
	std::mutex queueMutex;
	std::condition_variable queueUpdateCondition;

	std::condition_variable collisionsTested;

	bool shouldTerminate = false;
	unsigned int busyThreads = 0;

	void ThreadLoop();

private:
	Octant* root;

	void InsertObject(Octant* pOctant, ColliderObject* pObj);
	void BuildTree(Octant* pCurrent, const Vec3 extent, const unsigned int depth, const unsigned int maxDepth);
	void TestAllCollisions(Octant* pOctant);

	void DeleteChildren(Octant* pOctant);
	void ClearList(Octant* pOctant);

};