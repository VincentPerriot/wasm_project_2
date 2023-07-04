#version 300 es

layout(location = 0) in vec3 c_vertex; // Vertex position

out vec3 TexCoords;

uniform mat4 vp;

void main()
{
    TexCoords = vec3(c_vertex.x, -c_vertex.y, c_vertex.z);
    vec4 pos = vec4(c_vertex, 1.0) * vp;
    gl_Position = pos.xyww;
}  