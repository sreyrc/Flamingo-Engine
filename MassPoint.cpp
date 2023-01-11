#include "MassPoint.h"
#include <iostream>

void MassPoint::Update(float dt)
{
	// ---- Using RK4 integration ----
	// Calculate derivatives at different time steps, and average

	Derivative d, k1, k2, k3, k4;

	glm::vec3 acceleration = m_InverseMass * m_Force;

	k1 = { m_Velocity + d.dv * dt, acceleration };
	k2 = { m_Velocity + k1.dv * dt * 0.5f, acceleration };
	k3 = { m_Velocity + k2.dv * dt * 0.5f, acceleration };
	k4 = { m_Velocity + k3.dv * dt, acceleration };

	glm::vec3 dxdt = 1.0f / 6.0f *
		(k1.dp + 2.0f * (k2.dp + k3.dp) + k4.dp);

	glm::vec3 dvdt = 1.0f / 6.0f *
		(k1.dv + 2.0f * (k2.dv + k3.dv) + k4.dv);

	m_Position = m_Position + dxdt * dt;
	m_Velocity = m_Velocity + dvdt * dt;

	m_Force = glm::vec3(0);
}
