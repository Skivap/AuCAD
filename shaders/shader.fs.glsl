#version 330 core

// Uniforms
// uniform vec3 lightPos;
uniform vec3 camPos;

// In
struct VertexData {
    vec3 position;
    vec3 normal;
    vec3 color;
};
in VertexData vertexData;
// Out
out vec4 FragColor;

void main()
{
    vec3 lightPos = vec3(1.0, 0.0, 0.0);
    vec3 color = vertexData.color;

    // Ambient light
    vec3 ambient = 0.05 * color;

    // Diffuse light
    vec3 lightDir = normalize(lightPos - vertexData.position);
    vec3 normal = normalize(vertexData.normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // Specular Light (using Blinn-Phong)
    vec3 viewDir = normalize(camPos - vertexData.position);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec;

    // Final color
    FragColor = vec4(ambient + diffuse + specular, 1.0);
    // FragColor = vec4(vertexData.normal, 1.0);
}
