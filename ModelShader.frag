#version 330 core

in vec3 Normal;
in vec3 FragPos;

const int MAX_N_LIGHTS = 100;

struct Light {
	vec3 position;
	vec3 color;
};

struct Material {
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform float constant;
uniform float linear;
uniform float quadratic;

uniform Material material;
uniform Light lights[MAX_N_LIGHTS];

uniform vec3 viewPos;

uniform int numberOfLights;

out vec4 FragColor;

void main() {
	vec3 result = vec3(0);
	for(int i = 0; i < numberOfLights; i++) {
		vec3 ambient = vec3(0.3f) * lights[i].color;

		vec3 norm = normalize(Normal);

		vec3 lightDir = lights[i].position - FragPos;
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = diff * material.diffuse * lights[i].color;

		vec3 viewDir = viewPos - FragPos;
		vec3 halfwayDir = normalize(viewDir + lightDir);
		float spec = pow(max(dot(halfwayDir, norm), 0.0f), 64.0f);
		vec3 specular = spec * material.specular * lights[i].color;

		result += (ambient + diffuse + specular); 

	}
	FragColor = vec4(result, 1.0f);
	//FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
