#version 330 core
out vec4 FragColor;

in vec4 position;
uniform sampler2D tex;

const float pi = 3.14159;
void main()
{
	vec2 uv = vec2(0.5 - atan(position.y, position.x) / (2 * pi), acos(position.z) / pi);
	vec3 fragCol = texture(tex, uv).xyz;
	gl_FragData[0] = vec4(fragCol,1.0);
}