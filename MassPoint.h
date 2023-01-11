#pragma once

#include <glm/glm.hpp>
#include <utility>

struct Derivative {
	glm::vec3 dp, dv;

	Derivative() : dp(glm::vec3(0)), dv(glm::vec3(0)) {};
	Derivative(glm::vec3 _dp, glm::vec3 _dv) : dp(_dp), dv(_dv) {}
};

class MassPoint {

public:
	MassPoint() : m_Position(glm::vec3(0)), m_InverseMass(1.0f), 
		m_Force(glm::vec3(0)), m_Velocity(glm::vec3(0)), m_t(0) {}
	MassPoint(float x, float y, float z) : m_Position(glm::vec3(x, y, z)), 
		m_InverseMass(1.0f), m_Force(glm::vec3(0)), m_Velocity(glm::vec3(0)), m_t(0) {}

	void SetVelocity(glm::vec3 velocity) { m_Velocity = velocity; }
	void SetPosition(glm::vec3 position) { m_Position = position; }
	void SetPosition(float x, float y, float z) { m_Position = glm::vec3(x, y, z); }

	inline glm::vec3 GetVelocity() { return m_Velocity; }
	inline glm::vec3 GetPosition() { return m_Position; }
	inline float GetInverseMass() { return m_InverseMass; }

	void AddForce(glm::vec3 force) { m_Force += force; }

	void Update(float dt);

	glm::vec3 m_Position;

private:

	glm::vec3 m_Velocity;
	glm::vec3 m_Force;
	float m_InverseMass = 10;
	float m_t;
};
