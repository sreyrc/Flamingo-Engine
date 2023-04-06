#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;
out vec4 Position;

uniform mat4 model, view, proj;

void main() {
	Normal = transpose(inverse(mat3(model))) * aNormal;
	FragPos = vec3(model * vec4(aPos, 1.0f));
	Position = vec4(aPos, 1.0);
	gl_Position = proj * view * model * vec4(aPos, 1.0);
}
