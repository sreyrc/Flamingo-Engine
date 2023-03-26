#pragma once

#include <vector>

#include "Object.h"

struct CollisionObject {
	Collider* m_ColliderA, *m_ColliderB;

	CollisionObject() 
		: m_ColliderA(nullptr), m_ColliderB(nullptr) {}

	CollisionObject(Collider* colA, Collider* colB)
		: m_ColliderA(colA), m_ColliderB(colB) {}
};


class CollisionWorld
{
	typedef bool(*colFnPtr)(BoundingVolume*, BoundingVolume*);
public:
	CollisionWorld();

	void AddCollider(Collider* col);

	// Add objects which have colliders
	void Configure(std::vector<Object*>& objects);

	// Checks for collisions
	void Update();

	void ClearCollisionQueues();

private:
	std::vector<Collider*> m_Colliders;

	// 2D array of function pointers. Different functions according 
	// to typs of BVs involved
	std::vector<std::vector<colFnPtr>> isColliding;

	// All collisions where level 1 bounding volumes collide
	std::vector<CollisionObject> m_CollisionQueueLevel1;

	unsigned int m_NumCollisionsThisFrame;
};

