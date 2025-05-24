#version 330 core

// Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

// Uniform
uniform mat4 projection;
uniform mat4 view;
uniform mat4 viewport;

struct VertexData {
    vec3 color;
    vec3 normal;
};
out VertexData vertexData;

void main()
{
    vertexData.normal = normal;
    vertexData.color = color;
    gl_Position = viewport * projection * view * vec4(position, 1.0);
}
