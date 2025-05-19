#version 330 core

struct VertexData {
    vec3 color;
};
in VertexData vertexData;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexData.color, 1.0);
}
