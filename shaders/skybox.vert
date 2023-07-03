#version 300 es

layout(location = 0) in vec3 c_vertex; // Vertex position

out vec3 TexCoords;

uniform mat4 mvp;

void main()
{
    TexCoords = c_vertex;
    gl_Position = vec4(c_vertex, 1.0) * mvp;
}  