#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gRoughMetal;

in vec3 FragPos;
in vec3 Normal;

struct Material {
	vec3 kD;
	float alpha;
	float metalness;
};

uniform Material material;

void main() {
	gPosition = vec4(FragPos, 1.0f);
	gNormal = vec4(normalize(Normal), 1.0f); 
	gDiffuse = vec4(material.kD, 1.0f);
	gRoughMetal = vec4(material.alpha, material.metalness, 0.0, 1.0f);
}