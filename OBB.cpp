#include "OBB.h"
#include "Object.h"


// Vertices for drawing
void FillOBBVertexValues(std::vector<glm::vec3>& vertices, glm::vec3 min, glm::vec3 max) {

	vertices[0] = min;
	vertices[1] = glm::vec3(min.x + fabs(max.x - min.x), min.y, min.z);
	vertices[2] = glm::vec3(min.x, min.y + fabs(max.y - min.y), min.z);
	vertices[3] = glm::vec3(min.x, min.y, min.z + fabs(max.z - min.z));
	vertices[4] = glm::vec3(min.x + fabs(max.x - min.x), min.y + fabs(max.y - min.y), min.z);
	vertices[5] = glm::vec3(min.x + fabs(max.x - min.x), min.y, min.z + fabs(max.z - min.z));
	vertices[6] = glm::vec3(min.x, min.y + fabs(max.y - min.y), min.z + fabs(max.z - min.z));
	vertices[7] = max;
}

OBB::OBB()
	: m_Min(0), m_Max(0) {

	m_Type = BVType::OBB;

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	m_Indices.resize(24);

	m_Indices[0] = 0;  m_Indices[1] = 1;
	m_Indices[2] = 0;  m_Indices[3] = 2;
	m_Indices[4] = 0;  m_Indices[5] = 3;
	m_Indices[6] = 5;  m_Indices[7] = 1;
	m_Indices[8] = 5;  m_Indices[9] = 3;
	m_Indices[10] = 5; m_Indices[11] = 7;
	m_Indices[12] = 6; m_Indices[13] = 2;
	m_Indices[14] = 6; m_Indices[15] = 3;
	m_Indices[16] = 6; m_Indices[17] = 7;
	m_Indices[18] = 4; m_Indices[19] = 2;
	m_Indices[20] = 4; m_Indices[21] = 1;
	m_Indices[22] = 4; m_Indices[23] = 7;

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void OBB::Initialize()
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

	FillOBBVertexValues(m_VerticesInit, m_Min, m_Max);
}

void OBB::Update()
{
	// TODO: Recalculate only when world transform changes

	auto worldTransform = m_ParentCollider->GetParent()->
		GetComponent<Transform*>()->GetWorldTransform();

	// Transform initial points - if same as prev vec - no updates
	if (glm::vec3(worldTransform * glm::vec4(m_VerticesInit[0], 1.0f)) 
		== m_Vertices[0]) { return; }

	// -- if this transformation is diff from prev frame
	// 
	// Transform the vertices of the OBB to world space according to tranform of GO
	for (int i = 0; i < m_VerticesInit.size(); i++) {
		m_Vertices[i] = glm::vec3(worldTransform * glm::vec4(m_VerticesInit[i], 1.0f));
	}

	// Fill in the new data for drawing
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3),
		&m_Vertices[0], GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
}

void OBB::Draw(Shader* shader)
{
	if (m_InCollision) { shader->SetVec3("lineColor", glm::vec3(1.0f, 1.0f, 0.0f)); }
	else shader->SetVec3("lineColor", glm::vec3(1.0f, 1.0f, 1.0f));

	glLineWidth(1.5f);
	glBindVertexArray(m_VAO);
	glDrawElements(GL_LINES,
		static_cast<unsigned int>(m_Indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

glm::vec3 OBB::Support(glm::vec3 direction)
{
	glm::vec3 maxPoint{};
	float   maxDistance = -FLT_MAX;

	// Go through all the vertices - 
	// find point which has max dot product with the given dir i.e
	// largest and longest projection along that dir
	for (glm::vec3 vertex : m_Vertices) {
		float distance = glm::dot(vertex, direction);
		if (distance > maxDistance) {
			maxDistance = distance;
			maxPoint = vertex;
		}
	}

	return maxPoint;
}

