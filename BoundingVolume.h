#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glad/glad.h>
#include <vector>

class Collider;
class Shader;

enum class BVType {
	AABB,
	SPHERE,
	OBB,
	KDOP,
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
	unsigned int m_VAO = 0, m_VBO = 0, m_IBO = 0;

	bool m_InCollision = false;

public:
	inline BVType GetType() { return m_Type; };
	inline void SetParentCollider(Collider* par) { m_ParentCollider = par; };
	inline void IsInCollision(bool inCol) { m_InCollision = inCol; }
	inline bool InCollision() { return m_InCollision; }
	inline std::vector<glm::vec3> GetVertices() { return m_Vertices; }
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw(Shader* shader) = 0;

	// Returns the vertex of the collider furthest along the given dir
	virtual glm::vec3 Support(glm::vec3 direction) = 0;
};

