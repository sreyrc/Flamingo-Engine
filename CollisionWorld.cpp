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

struct Simplex {
private:
	std::array<glm::vec3, 4> m_points;
	unsigned m_size;

public:
	Simplex()
		: m_points({ glm::vec3(0), glm::vec3(0), 
			glm::vec3(0), glm::vec3(0) })
		, m_size(0)
	{}

	Simplex& operator=(std::initializer_list<glm::vec3> list) {
		for (auto v = list.begin(); v != list.end(); v++) {
			m_points[std::distance(list.begin(), v)] = *v;
		}
		m_size = list.size();

		return *this;
	}

	void push_front(glm::vec3 point) {
		m_points = { point, m_points[0], m_points[1], m_points[2] };
		m_size = std::min(m_size + 1, 4u);
	}

	glm::vec3& operator[](unsigned i) { return m_points[i]; }
	unsigned size() const { return m_size; }

	auto begin() const { return m_points.begin(); }
	auto end()   const { return m_points.end() - (4 - m_size); }
};


bool SameDirection(
	const glm::vec3& direction,
	const glm::vec3& ao)
{
	return glm::dot(direction, ao) > 0;
}

bool Line(Simplex& points, glm::vec3& direction)
{
	glm::vec3 a = points[0];
	glm::vec3 b = points[1];

	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;

	if (SameDirection(ab, ao)) {
		direction = glm::cross(ab, glm::cross(ao, ab));
	}

	else {
		points = { a };
		direction = ao;
	}

	return false;
}

bool Triangle(
	Simplex& points,
	glm::vec3& direction)
{
	glm::vec3 a = points[0];
	glm::vec3 b = points[1];
	glm::vec3 c = points[2];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ao = -a;

	glm::vec3 abc = glm::cross(ab, ac);

	if (SameDirection(glm::cross(abc, ac), ao)) {
		if (SameDirection(ac, ao)) {
			points = { a, c };
			direction = glm::cross(ac, glm::cross(ao, ac));
		}

		else return Line(points = { a, b }, direction);
	}

	else {
		if (SameDirection(glm::cross(ab, abc), ao)) {
			return Line(points = { a, b }, direction);
		}

		else {
			if (SameDirection(abc, ao)) {
				direction = abc;
			}

			else {
				points = { a, c, b };
				direction = -abc;
			}
		}
	}
	return false;
}

bool Tetrahedron(
	Simplex& points,
	glm::vec3& direction)
{
	glm::vec3 a = points[0];
	glm::vec3 b = points[1];
	glm::vec3 c = points[2];
	glm::vec3 d = points[3];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;
	glm::vec3 ao = -a;

	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);

	if (SameDirection(abc, ao)) {
		return Triangle(points = { a, b, c }, direction);
	}

	if (SameDirection(acd, ao)) {
		return Triangle(points = { a, c, d }, direction);
	}

	if (SameDirection(adb, ao)) {
		return Triangle(points = { a, d, b }, direction);
	}

	return true;
}


bool NextSimplex(
	Simplex& points,
	glm::vec3& direction)
{
	switch (points.size()) {
		case 2: return Line(points, direction);
		case 3: return Triangle(points, direction);
		case 4: return Tetrahedron(points, direction);
	}

	// never should be here
	return false;
}



bool GJK(BoundingVolume* bvA, BoundingVolume* bvB) {

	// Intial support point
	glm::vec3 support = bvA->Support(glm::vec3(1, 0, 0)) - 
		bvB->Support(-glm::vec3(1, 0, 0));

	// Simplex is an array of points, max count is 4
	Simplex points;
	points.push_front(support);

	// New direction is towards the origin
	glm::vec3 direction = -support;

	while (true) {
		support = bvA->Support(direction) -
			bvB->Support(-direction);

		if (glm::dot(support, direction) <= 0) {
			return false; // no collision
		}

		points.push_front(support);

		if (NextSimplex(points, direction)) {
			return true;
		}
	}
}

CollisionWorld::CollisionWorld() 
	: m_NumCollisionsThisFrame(0)
{
	isColliding.resize(static_cast<int>(BVType::NUM));

	for (int i = 0; i < isColliding.size(); i++) {
		isColliding[i].resize(static_cast<int>(BVType::NUM));
	}

	isColliding[static_cast<int>(BVType::AABB)]
		[static_cast<int>(BVType::AABB)] = &AABBAABB;

	isColliding[static_cast<int>(BVType::OBB)]
		[static_cast<int>(BVType::OBB)] = &GJK;

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

	// TODO: Doing brute force collision checks rn. 
	// To be optimized with spatial partitioning soon
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
