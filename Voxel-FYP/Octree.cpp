#include "Octree.h"
#include "Structures.h"
#include "Simulator.h"

#include <cmath>
#include <thread>
#include <iostream>

void Octree::ThreadLoop()
{
	while (!shouldTerminate)
	{
		std::unique_lock<std::mutex> lock(queueMutex);

		queueUpdateCondition.wait(lock, [this]() {
			return !octantQueue.empty() || shouldTerminate;
			});

		if (!octantQueue.empty())
		{
			++busyThreads;
			Octant* task = octantQueue.front();
			octantQueue.pop();

			lock.unlock();
			task->TestCollisions();
			lock.lock();

			--busyThreads;
			collisionsTested.notify_one();
		}
	}
}

void Octree::InsertObject(Octant* pOctant, ColliderObject* pObj)
{
	unsigned int index = 0;
	bool straddle = false;
	for (int i = 0; i < 3; i++) // each axis
	{
		// get distance between octant's centre and objects position
		// as octants centre is where the split will be on that axis
		// if the distance to the axis split is smaller than the
		// half size of the object, then the object straddles that
		// splitting axis, so stop checking other axes
		float delta = pObj->data->position[i] - pOctant->centre[i];
		if (abs(delta) <= pObj->data->position.w)
		{
			straddle = true;
			break;
		}
		if (delta > 0.0f) index |= (1 << i); // the side the object is on affects index
	}

	// if not straddling and child exists insert deeper
	if (!straddle && pOctant->children[index])
	{
		InsertObject(pOctant->children[index], pObj);
	}
	else
	{
		// if there is no more children or the object is
		// straddling an axis, add to current octants list
		pOctant->AddToList(pObj);
	}
}

void Octree::BuildTree(Octant* pCurrent, const Vec3 extent, const unsigned int depth, const unsigned int maxDepth)
{
	Vec3 halfExtent = extent / 2.0f;
	Vec3 offset;

	for (unsigned int i = 0; i < 8; i++)
	{
		offset.x = ((i & 1) ? halfExtent.x : -halfExtent.x);
		offset.y = ((i & 2) ? halfExtent.y : -halfExtent.y);
		offset.z = ((i & 4) ? halfExtent.z : -halfExtent.z);
		pCurrent->children[i] = new Octant(pCurrent->centre + offset, pCurrent);

		if (depth != maxDepth)
		{
			BuildTree(pCurrent->children[i], halfExtent, depth + 1, maxDepth);
		}
	}
}

void Octree::TestAllCollisions(Octant* pOctant)
{
	{
		std::lock_guard<std::mutex> guard(queueMutex);
		octantQueue.push(pOctant);
	}
	queueUpdateCondition.notify_one();
	
	for (int i = 0; i < 8; i++)
	{
		Octant* child = pOctant->children[i];
		if (child != nullptr)
		{
			TestAllCollisions(child);
		}
	}
}

void Octree::DeleteChildren(Octant* pOctant)
{
	for (Octant*& child : pOctant->children)
	{
		if (child != nullptr)
		{
			DeleteChildren(child);
			delete child;
			child = nullptr;
		}
	}
}

void Octree::ClearList(Octant* pOctant)
{
	for (Octant*& child : pOctant->children)
	{
		if (child != nullptr)
		{
			ClearList(child);
		}

		pOctant->ClearList();
	}
}

Octree::Octree(const Vec3 position, const Vec3 extent, const unsigned int maxDepth)
{
	root = new Octant(position, nullptr);
	if (maxDepth != 0)
	{
		BuildTree(root, extent, 1, maxDepth);
	}
	
	threads.resize(8);
	for (std::thread& thread : threads)
	{
		thread = std::thread(&Octree::ThreadLoop, this);
	}
}

Octree::~Octree()
{
	{
		std::lock_guard<std::mutex> guard(queueMutex);
		shouldTerminate = true;
	}
	queueUpdateCondition.notify_all();

	for (std::thread& thread : threads)
	{
		thread.join();
	}

	DeleteChildren(root);
	delete root;
}

void Octree::Insert(ColliderObject* pObj)
{
	InsertObject(root, pObj);
}

void Octree::TestCollisions()
{
	TestAllCollisions(root);

	std::unique_lock<std::mutex> lock(queueMutex);
	collisionsTested.wait(lock, [this]() { return octantQueue.empty() && (busyThreads == 0); });
}

void Octree::ClearLists()
{
	ClearList(root);
}

Octree::Octant::Octant(Vec3 centre, Octant* parent) :
	centre(centre)
{
	children = std::array<Octant*, 8>();
	for (Octant*& child : children)
	{
		child = nullptr;
	}
	pObjects = nullptr;
	pParent = parent;
}

void Octree::Octant::AddToList(ColliderObject* pObj)
{
	std::lock_guard<std::mutex> guard(listMutex);
	pObj->pNext = pObjects;
	pObjects = pObj;
}

void Octree::Octant::TestCollisions()
{
	std::array<Octant*, 6> others{};
	unsigned int otherCount = 0;

	for (Octant* other = this; other; other = other->pParent)
	{
		others[otherCount++] = other;
	}
	unsigned int maxOthers = otherCount;

	for (unsigned int index = 0; otherCount != 0; index = (index+1) % maxOthers)
	{
		if (others[index] == nullptr) continue;

		std::lock_guard<std::mutex> guard{ listMutex };
		Octant* other = others[index];

		ColliderObject* objA, * objB;
		if (this != other)
		{
			std::unique_lock<std::mutex> lock{ other->listMutex, std::defer_lock };
			if (lock.try_lock())
			{
				for (objA = other->pObjects; objA; objA = objA->pNext)
				{
					for (objB = pObjects; objB; objB = objB->pNext)
					{
						Simulator::TestCollision(*objA->data, *objB->data);
					}
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			for (objA = other->pObjects; objA; objA = objA->pNext)
			{
				for (objB = objA->pNext; objB; objB = objB->pNext) // only check each once if is same octant
				{
					Simulator::TestCollision(*objA->data, *objB->data);
				}
			}
		}

		others[index] = nullptr;
		--otherCount;
	}
}

void Octree::Octant::ClearList()
{
	std::lock_guard<std::mutex> guard(listMutex);
	pObjects = nullptr;
}