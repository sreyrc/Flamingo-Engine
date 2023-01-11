#include "AABB.h"
#include "Object.h"


// Vertices for drawing
void FillAABBVertexValues(std::vector<glm::vec3>& vertices, glm::vec3 min, glm::vec3 max) {

	vertices[0] = min;
	vertices[1] = glm::vec3(min.x + fabs(max.x - min.x), min.y, min.z);
	vertices[2] = glm::vec3(min.x, min.y + fabs(max.y - min.y), min.z);
	vertices[3] = glm::vec3(min.x, min.y, min.z + fabs(max.z - min.z));
	vertices[4] = glm::vec3(min.x + fabs(max.x - min.x), min.y + fabs(max.y - min.y), min.z);
	vertices[5] = glm::vec3(min.x + fabs(max.x - min.x), min.y, min.z + fabs(max.z - min.z));
	vertices[6] = glm::vec3(min.x, min.y + fabs(max.y - min.y), min.z + fabs(max.z - min.z));
	vertices[7] = max;
}

void AABB::Initialize()
{
	m_Vertices.clear();
	m_VerticesInit.clear();

	auto mesh = m_ParentCollider->GetParent()->GetComponent<ModelComp*>();
	auto vertices = mesh->GetModel()->GetAllVertices();

	m_Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	m_Min = { FLT_MAX, FLT_MAX, FLT_MAX };

	for (const auto& v : vertices) {
		if (v.Position.x > m_Max.x) { m_Max.x = v.Position.x; }
		if (v.Position.y > m_Max.y) { m_Max.y = v.Position.y; }
		if (v.Position.z > m_Max.z) { m_Max.z = v.Position.z; }

		if (v.Position.x < m_Min.x) { m_Min.x = v.Position.x; }
		if (v.Position.y < m_Min.y) { m_Min.y = v.Position.y; }
		if (v.Position.z < m_Min.z) { m_Min.z = v.Position.z; }
	}

	m_VerticesInit.resize(8);
	m_Vertices.resize(8);

	FillAABBVertexValues(m_VerticesInit, m_Min, m_Max);
}

void AABB::Update()
{
	// TODO: Recalculate only when world transform changes

	auto worldTransform = m_ParentCollider->GetParent()->
		GetComponent<Transform*>()->GetWorldTransform();

	// Transform the vertices of the AABB to world space according to tranform of GO
	for (int i = 0; i < m_VerticesInit.size(); i++) {
		m_Vertices[i] = glm::vec3(worldTransform * glm::vec4(m_VerticesInit[i], 1.0f));
	}

	// Now find a new AABB from the transformed AABB.
	m_Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	m_Min = { FLT_MAX, FLT_MAX, FLT_MAX };

	for (const auto& v : m_Vertices) {
		if (v.x > m_Max.x) { m_Max.x = v.x; }
		if (v.y > m_Max.y) { m_Max.y = v.y; }
		if (v.z > m_Max.z) { m_Max.z = v.z; }

		if (v.x < m_Min.x) { m_Min.x = v.x; }
		if (v.y < m_Min.y) { m_Min.y = v.y; }
		if (v.z < m_Min.z) { m_Min.z = v.z; }
	}

	FillAABBVertexValues(m_Vertices, m_Min, m_Max);
}

void AABB::Draw()
{
	glLineWidth(1.5f);

	glBegin(GL_LINES);
		glVertex3f(m_Vertices[0].x, m_Vertices[0].y, m_Vertices[0].z);
		glVertex3f(m_Vertices[1].x, m_Vertices[1].y, m_Vertices[1].z);

		glVertex3f(m_Vertices[0].x, m_Vertices[0].y, m_Vertices[0].z);
		glVertex3f(m_Vertices[2].x, m_Vertices[2].y, m_Vertices[2].z);

		glVertex3f(m_Vertices[0].x, m_Vertices[0].y, m_Vertices[0].z);
		glVertex3f(m_Vertices[3].x, m_Vertices[3].y, m_Vertices[3].z);

		glVertex3f(m_Vertices[5].x, m_Vertices[5].y, m_Vertices[5].z);
		glVertex3f(m_Vertices[1].x, m_Vertices[1].y, m_Vertices[1].z);

		glVertex3f(m_Vertices[5].x, m_Vertices[5].y, m_Vertices[5].z);
		glVertex3f(m_Vertices[3].x, m_Vertices[3].y, m_Vertices[3].z);

		glVertex3f(m_Vertices[5].x, m_Vertices[5].y, m_Vertices[5].z);
		glVertex3f(m_Vertices[7].x, m_Vertices[7].y, m_Vertices[7].z);

		glVertex3f(m_Vertices[6].x, m_Vertices[6].y, m_Vertices[6].z);
		glVertex3f(m_Vertices[2].x, m_Vertices[2].y, m_Vertices[2].z);

		glVertex3f(m_Vertices[6].x, m_Vertices[6].y, m_Vertices[6].z);
		glVertex3f(m_Vertices[3].x, m_Vertices[3].y, m_Vertices[3].z);

		glVertex3f(m_Vertices[6].x, m_Vertices[6].y, m_Vertices[6].z);
		glVertex3f(m_Vertices[7].x, m_Vertices[7].y, m_Vertices[7].z);

		glVertex3f(m_Vertices[4].x, m_Vertices[4].y, m_Vertices[4].z);
		glVertex3f(m_Vertices[2].x, m_Vertices[2].y, m_Vertices[2].z);

		glVertex3f(m_Vertices[4].x, m_Vertices[4].y, m_Vertices[4].z);
		glVertex3f(m_Vertices[1].x, m_Vertices[1].y, m_Vertices[1].z);

		glVertex3f(m_Vertices[4].x, m_Vertices[4].y, m_Vertices[4].z);
		glVertex3f(m_Vertices[7].x, m_Vertices[7].y, m_Vertices[7].z);
	glEnd();
}
