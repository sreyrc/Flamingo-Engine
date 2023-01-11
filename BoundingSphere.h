#pragma once
#include "BoundingVolume.h"

class BoundingSphere : public BoundingVolume
{
public:
	BoundingSphere() : 
		m_Center(0), m_Radius(0) { m_Type = BVType::SPHERE; }

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

	inline glm::vec3 GetCenter() { return m_Center; }
	inline float GetRadius() { return m_Radius; }

private:
	//MostSeperatedPointsOnABBB

	glm::vec3 m_Center;
	float m_Radius;
};

