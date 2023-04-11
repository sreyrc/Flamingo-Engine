#version 330 core

layout (location = 0) out vec4 g_Position;
layout (location = 1) out vec4 g_Normal;
layout (location = 2) out vec4 g_Diffuse;
layout (location = 3) out vec4 g_RoughMetal;

in vec3 FragPos;
in vec3 Normal;
in vec4 Position;

struct Material {
	vec3 kD;
	float alpha;
	float metalness;
};

uniform bool isSkyDome;
uniform Material material;
uniform float expControl;

uniform sampler2D skyBoxTexture;

const float PI = 3.14159;

void main() {
	if (isSkyDome) {
		vec2 uv = vec2(0.5f - (atan(Position.z, Position.x) / (2 * PI)), acos(Position.y) / PI);
		g_Normal = vec4(Normal, 1.0f);  
		vec3 diffuse = texture(skyBoxTexture, uv).xyz;
        diffuse = pow(diffuse, vec3(2.2));
		g_Diffuse = vec4(diffuse, -1.0);
		g_RoughMetal = vec4(1.0f, 0.0f, 0.0, 1.0f);
	}
	else {
		g_Position = vec4(FragPos, 1.0f);
		g_Normal = vec4(normalize(Normal), 1.0f); 
		g_Diffuse = vec4(material.kD, 1.0f);
		g_RoughMetal = vec4(material.alpha, material.metalness, 0.0, 1.0f);
	}
}