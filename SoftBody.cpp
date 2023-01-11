#include "SoftBody.h"

#include <iostream>

SoftBody::SoftBody(int m, int n, int l)  : m_VAO(0), m_VBO(0), m_IBO(0)
{
	m_IndicesForDrawing.clear();

	// Create mass points with initial configurations for this softbody
	for (int z = 0; z <= l; z++) {
		for (int y = 0; y <= n; y++) {
			for (int x = 0; x <= m; x++) {
				
				m_MassPoints[std::make_tuple(x, y, z)] =
					new MassPoint(x * 0.3f + 1, y * 0.3f + 1, z * 0.3f + 1);

				std::cout << x << ' ' << y << ' ' << z <<'\n';
			}
		}
	}

	int numMassPoints = m_MassPoints.size();

	int x, y, z;

	// Setup spring connections
	for (auto [key, val] : m_MassPoints) {
		int x = std::get<0>(key);
		int y = std::get<1>(key);
		int z = std::get<2>(key);


		// --- Springs for elastic forces --- 

		if (x + 1 <= m)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x + 1, y, z}]));

		if (y + 1 <= n)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x, y + 1, z}]));

		if (z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x, y, z + 1}]));

		
		// --- Springs for bend forces --- 

		if (x + 2 <= m)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x + 2, y, z}]));

		if (y + 2 <= n)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x, y + 2, z}]));

		if (z + 2 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x, y, z + 2}]));

		
		// --- Springs for shear forces --- 

		if (x + 1 <= m && y + 1 <= n)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x + 1, y + 1, z}]));

		if (y + 1 <= n && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x, y + 1, z + 1}]));

		if (x + 1 <= m && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x + 1, y, z + 1}]));

		if (x - 1 >= 0 && y + 1 <= n)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x - 1, y + 1, z}]));

		if (y - 1 >= 0 && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x, y - 1, z + 1}]));

		if (x - 1 >= 0 && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x - 1, y, z + 1}]));

		if (x + 1 <= m && y + 1 <= n && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x + 1, y + 1, z + 1}]));

		if (x - 1 >= 0 && y - 1 >= 0 && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x - 1, y - 1, z + 1}]));

		if (x + 1 <= m && y - 1 >= 0 && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x + 1, y - 1, z + 1}]));

		if (x - 1 >= 0 && y + 1 <= n && z + 1 <= l)
			m_Springs.push_back(new Spring(m_MassPoints[key], m_MassPoints[{x - 1, y + 1, z + 1}]));

	}

	m_AnchorPoints.first = new MassPoint(-0.5f, 1.5f, -0.5f);
	m_AnchorPoints.second = new MassPoint(3.5f, 1.5f, 3.5f);


	// Link anchor points to two points on the soft body
	m_Springs.push_back(new Spring(m_MassPoints[{0, n, 0}], m_AnchorPoints.first));
	m_Springs.push_back(new Spring(m_MassPoints[{m, n, l}], m_AnchorPoints.second));


	GenerateIndices(m, n, l);

	m_Ks = 100.0f;

	glGenVertexArrays(1, &m_VAO);

	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);
}

void printVec3(glm::vec3 vec3) {
	std::cout << vec3.x << ' ' << vec3.y << ' ' << vec3.z << '\n';
}

void SoftBody::Update(float dt)
{
	size_t size = m_Springs.size();
	
	// Apply spring forces on mass points based on spring forces using Hooke's law
	for (int i = 0; i < size; i++) {
		
		MassPoint* mpA = m_Springs[i]->a;
		MassPoint* mpB = m_Springs[i]->b;

		auto AminusB = mpA->GetPosition() - mpB->GetPosition();

		// Spring force on A
		mpA->AddForce(-m_Ks * (AminusB / glm::length(AminusB)) *
			(glm::length(AminusB) - m_Springs[i]->L0));

		// Spring force on B
		mpB->AddForce(-m_Ks * (-AminusB / glm::length(AminusB)) *
			(glm::length(AminusB) - m_Springs[i]->L0));
		
		// Damping forces on A and B
		mpA->AddForce(-0.5f * (mpA->GetVelocity() - mpB->GetVelocity()));
		mpB->AddForce(-0.5f * (mpB->GetVelocity() - mpA->GetVelocity()));

		// Apply gravity
		mpA->AddForce({ 0, -0.0098f, 0 });
		mpB->AddForce({ 0, -0.0098f, 0 });
	}

	// Update positions of mass points
	for (auto mP : m_MassPoints) {
		mP.second->Update(dt);
	}
}

void SoftBody::UpdateVertexDataAndDraw()
{
	// Generate and load new buffer data for this frame and draw the object

	// TODO: Don't clear and add every frame. Set size earlier and set data
	m_BufferDataForDrawing.clear();
	
	// Load vertex data of all mass points into a vector
	for (auto& mp : m_MassPoints) {

		VertexData data;
		data.pos = mp.second->GetPosition();
		data.normal = glm::vec3(-1, 0, 0);

		m_BufferDataForDrawing.push_back(data);
	}
	 
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_BufferDataForDrawing.size() * sizeof(VertexData), &m_BufferDataForDrawing[0], GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndicesForDrawing.size() * sizeof(unsigned int), &m_IndicesForDrawing[0], GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));


	glDrawElements(GL_TRIANGLES, m_IndicesForDrawing.size(), GL_UNSIGNED_INT, 0);
}

void SoftBody::GenerateIndices(int m, int n, int l)
{
	// Faces for rendering 
	
	// Back face
	for (int y = 0; y < n; y++) {
		for (int x = 0; x < m; x++) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, y, 0 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x + 1, y, 0 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x + 1, y + 1, 0 })));
		}
	}

	for (int y = n; y > 0; y--) {
		for (int x = m; x > 0; x--) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, y, 0 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x - 1, y, 0 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x - 1, y - 1, 0 })));
		}
	}


	// Front face
	for (int y = 0; y < n; y++) {
		for (int x = 0; x < m; x++) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, y, l })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x + 1, y, l })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x + 1, y + 1, l })));
		}
	}

	for (int y = n; y > 0; y--) {
		for (int x = m; x > 0; x--) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, y, l })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x - 1, y, l })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x - 1, y - 1, l })));
		}
	}

	// Left face
	for (int y = 0; y < n; y++) {
		for (int z = 0; z < l; z++) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ 0, y, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ 0, y, z + 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ 0, y + 1, z + 1 })));
		}
	}

	for (int y = n; y > 0; y--) {
		for (int z = l; z > 0; z--) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ 0, y, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ 0, y, z - 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ 0, y - 1, z - 1 })));
		}
	}

	// Right face
	for (int y = 0; y < n; y++) {
		for (int z = 0; z < l; z++) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ m, y, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ m, y, z + 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ m, y + 1, z + 1 })));
		}
	}

	for (int y = n; y > 0; y--) {
		for (int z = l; z > 0; z--) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ m, y, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ m, y, z - 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ m, y - 1, z - 1 })));
		}
	}

	// Bottom face
	for (int z = 0; z < l; z++) {
		for (int x = 0; x < m; x++) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, 0, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, 0, z + 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x + 1, 0, z + 1 })));
		}
	}

	for (int z = l; z > 0; z--) {
		for (int x = m; x > 0; x--) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, 0, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x - 1, 0, z - 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, 0, z - 1 })));
		}
	}

	// Top face
	for (int z = 0; z < l; z++) {
		for (int x = 0; x < m; x++) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, n, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, n, z + 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x + 1, n, z + 1 })));
		}
	}

	for (int z = l; z > 0; z--) {
		for (int x = m; x > 0; x--) {
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, n, z })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x - 1, n, z - 1 })));
			m_IndicesForDrawing.push_back(std::distance(std::begin(m_MassPoints), m_MassPoints.find({ x, n, z - 1 })));
		}
	}

}
