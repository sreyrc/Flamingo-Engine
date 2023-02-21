#version 330 core

layout (location = 0) out vec4 g_LightDepth;

in vec4 position;

void main() {
//	float ndc_depth = position.z / position.w;
//	float far = gl_DepthRange.far; 
//	float near = gl_DepthRange.near;
//
//	float z = (((far-near) * ndc_depth) + near + far) / 2.0;
//	g_LightDepth = vec4(z, z*z, z*z*z, z*z*z*z);
	g_LightDepth = position;
}
