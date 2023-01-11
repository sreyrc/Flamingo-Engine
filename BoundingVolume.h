#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glad/glad.h>
#include <vector>

class Collider;

enum class BVType {
	AABB,
	SPHERE,
	NUM
};

// Base class for Bounding Volume
class BoundingVolume
{
protected:
	// Type of the bounding box
	BVType m_Type;

	// Vertices of the bounding volume if needed
	std::vector<glm::vec3> m_Vertices;

	// Reference to parent collider
	Collider* m_ParentCollider;

	// For drawing
	GLuint VAO, VBO;

public:
	inline BVType GetType() { return m_Type; };
	void SetParentCollider(Collider* par) { m_ParentCollider = par; };
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
};

