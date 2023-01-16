#pragma once
#include "BoundingVolume.h"

class AABB : public BoundingVolume
{
public:
	AABB();
	virtual void Initialize();
	virtual void Update();
	virtual void Draw(Shader* shader);

	inline glm::vec3 GetMin() { return m_Min; }
	inline glm::vec3 GetMax() { return m_Max; }

private:
	glm::vec3 m_Min, m_Max;
	std::vector<glm::vec3> m_VerticesInit;
	std::vector<unsigned int> m_Indices;
};