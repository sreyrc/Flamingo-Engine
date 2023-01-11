#pragma once
#include "BoundingVolume.h"

class AABB : public BoundingVolume
{
public:
	AABB() : m_Min(0), m_Max(0) { m_Type = BVType::AABB; }
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

	inline glm::vec3 GetMin() { return m_Min; }
	inline glm::vec3 GetMax() { return m_Max; }

private:
	glm::vec3 m_Min, m_Max;
	std::vector<glm::vec3> m_VerticesInit;
};

