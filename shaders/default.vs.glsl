#version 330 core

// Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// Uniform
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct VertexData {
    vec3 color;
};
out VertexData vertexData;

void main()
{
    vertexData.color = color;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
