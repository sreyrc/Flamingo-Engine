#version 330 core

layout (location = 0) out vec4 g_Position;
layout (location = 1) out vec4 g_Normal;
layout (location = 2) out vec4 g_Diffuse;
layout (location = 3) out vec4 g_RoughMetal;

in vec3 FragPos;
in vec3 Normal;

struct Material {
	vec3 kD;
	float alpha;
	float metalness;
};

uniform Material material;

void main() {
	g_Position = vec4(FragPos, 1.0f);
	g_Normal = vec4(normalize(Normal), 1.0f); 
	g_Diffuse = vec4(material.kD, 1.0f);
	g_RoughMetal = vec4(material.alpha, material.metalness, 0.0, 1.0f);
}