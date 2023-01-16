#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gRoughMetal;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

in mat3 TBN;

uniform sampler2D normalMap;
uniform sampler2D diffuseMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;

void main() {
	gPosition = vec4(FragPos, 1.0f);

	vec3 normal = texture(normalMap, TexCoords).rgb;
	normal  = normal * 2.0f - 1.0f;
	gNormal = vec4(normalize(TBN * normal), 1.0f);

	gDiffuse = vec4(texture(diffuseMap, TexCoords).rgb, 1.0f);
		
	gRoughMetal = vec4(texture(roughnessMap, TexCoords).r, 
	texture(metallicMap, TexCoords).g, 0.0f, 1.0f);
}