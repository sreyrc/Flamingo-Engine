#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 position;

void main() {
	position = proj * view * model * vec4(aPos, 1.0f);
	gl_Position = position;
}
