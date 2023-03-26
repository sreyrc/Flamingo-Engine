#version 330 core

layout (location = 0) out vec4 g_LightDepth;

in vec4 position;

uniform float minDepth, maxDepth;

void main() {
	float z = (position.w - minDepth) / (maxDepth - minDepth);
	g_LightDepth = vec4(z, z*z, z*z*z, z*z*z*z);
}
