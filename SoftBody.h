#pragma once

#include "MassPoint.h"

#include <glad/glad.h>

#include <vector>
#include <tuple>
#include <map>

struct Spring {

	Spring(MassPoint* ma, MassPoint* mb)
		: a(ma), b(mb), ks(50.0f) 
	{
		L0 = glm::length(ma->GetPosition() - mb->GetPosition());
	}

	// Mass points connected together by a spring
	MassPoint* a, * b;

	// Spring constant
	float ks;

	// Intial length between mass points
	float L0;
};


struct VertexData {
	glm::vec3 pos;
	glm::vec3 normal;
};

class SoftBody {

public:

	SoftBody(int m, int n, int l);

	// Physics updates
	void Update(float dt);

	// Update the data in the buffer and draw skin
	void UpdateVertexDataAndDraw();

	std::map<std::tuple<int, int, int>, MassPoint*>& GetMassPoints() { return m_MassPoints; }
	inline std::vector<Spring*>& GetSprings() { return m_Springs; }
	inline std::pair<MassPoint*, MassPoint*>& GetAnchorPoints() { return m_AnchorPoints; }


	float m_Ks;

private:
	// Generate indices for skin drawing
	void GenerateIndices(int m, int n, int l);

	// The collection of mass points that make up this soft body
	std::map<std::tuple<int, int, int>, MassPoint*> m_MassPoints;

	// All spring connections
	std::vector<Spring*> m_Springs;

	// Anchor points for this soft body
	std::pair<MassPoint*, MassPoint*> m_AnchorPoints;

	// Containers holding data for skin drawing
	std::vector<VertexData> m_BufferDataForDrawing;
	std::vector<unsigned int> m_IndicesForDrawing;

	// Buffers for drawing the skin
	unsigned int m_VAO, m_VBO, m_IBO;
};