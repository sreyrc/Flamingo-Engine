#include "CollisionWorld.h"

#include "AABB.h"
#include "OBB.h"

#define NUM_MAX_COLLISIONS 1000

bool AABBAABB(BoundingVolume* bvA, BoundingVolume* bvB)
{
	AABB* aabbA = static_cast<AABB*>(bvA);
	AABB* aabbB = static_cast<AABB*>(bvB);

	glm::vec3 aMax = aabbA->GetMax();
	glm::vec3 aMin = aabbA->GetMin();

	glm::vec3 bMax = aabbB->GetMax();
	glm::vec3 bMin = aabbB->GetMin();

	// Exit with no intersection if separated along an axis
	if (aMax[0] < bMin[0] || aMin[0] > bMax[0]) return false;
	if (aMax[1] < bMin[1] || aMin[1] > bMax[1]) return false;
	if (aMax[2] < bMin[2] || aMin[2] > bMax[2]) return false;

	// Overlapping on all axes means AABBs are intersecting
	return true;
}

bool OBBOBB(BoundingVolume* bvA, BoundingVolume* bvB) {

	return false;
}

CollisionWorld::CollisionWorld()
{
	isColliding.resize(static_cast<int>(BVType::NUM));

	for (int i = 0; i < isColliding.size(); i++) {
		isColliding[i].resize(static_cast<int>(BVType::NUM));
	}

	isColliding[static_cast<int>(BVType::AABB)]
		[static_cast<int>(BVType::AABB)] = &AABBAABB;

	isColliding[static_cast<int>(BVType::OBB)]
		[static_cast<int>(BVType::OBB)] = &OBBOBB;

	m_CollisionQueueLevel1.resize(NUM_MAX_COLLISIONS);
}

void CollisionWorld::Configure(std::vector<Object*>& objects)
{
	m_Colliders.clear();
	for (auto& obj : objects) {
		auto col = obj->GetComponent<Collider*>();
		if (col) m_Colliders.push_back(col); 
	}
}

void CollisionWorld::Update()
{
	m_NumCollisionsThisFrame = 0;

	// TODO: Doing brute force collsion checks rn. To be optimized with 
	// spatial partitioning soon
	for (int i = 0; i < m_Colliders.size(); i++) {
		auto bvLevel1A = m_Colliders[i]->m_BVLevel1;

		for (int j = 0; j < m_Colliders.size(); j++) {

			if (i == j) continue;

			auto bvLevel1B = m_Colliders[j]->m_BVLevel1;

			if (isColliding[static_cast<int>(bvLevel1A->GetType())]
				[static_cast<int>(bvLevel1B->GetType())](bvLevel1A, bvLevel1B)) {

				m_Colliders[i]->m_BVLevel1->IsInCollision(true);
				m_Colliders[j]->m_BVLevel1->IsInCollision(true);

				m_CollisionQueueLevel1[m_NumCollisionsThisFrame++] =
					CollisionObject(m_Colliders[i], m_Colliders[j]);
			}
		}
	}

	for (int i = 0; i < m_NumCollisionsThisFrame; i++) {

		auto bvLevel2A = m_CollisionQueueLevel1[i].m_ColliderA->m_BVLevel2;
		auto bvLevel2B = m_CollisionQueueLevel1[i].m_ColliderB->m_BVLevel2;

		if (isColliding[static_cast<int>(bvLevel2A->GetType())]
			[static_cast<int>(bvLevel2B->GetType())](bvLevel2A, bvLevel2B)) {

			m_CollisionQueueLevel1[i].m_ColliderA->m_BVLevel2->IsInCollision(true);
			m_CollisionQueueLevel1[i].m_ColliderB->m_BVLevel2->IsInCollision(true);
		}
	}
}

void CollisionWorld::ClearCollisionQueues()
{
	for (int i = 0; i < m_NumCollisionsThisFrame; i++) {
		m_CollisionQueueLevel1[i].m_ColliderA->IsInCollision(false);
		m_CollisionQueueLevel1[i].m_ColliderB->IsInCollision(false);
	}
	//m_CollisionQueueLevel1.clear();
	// TODO: Don't push and clear repeatedly. 
	// Initialize with a fixed queue size
	m_NumCollisionsThisFrame = 0;

}
