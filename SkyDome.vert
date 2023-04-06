#version 330

layout (location = 0) in vec3 aPos;

uniform mat4 proj, view, world;

out vec4 position;

void main()
{
    gl_Position = proj * view * world * vec4(aPos, 1.0f);
    position = vec4(aPos, 1.0f);
}