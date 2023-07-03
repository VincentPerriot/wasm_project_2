#version 300 es

layout (location = 0) in vec4 a_vertex; // vertex position 
layout (location = 1) in vec3 a_colors; // vertex colors
layout (location = 2) in vec2 a_texcoord; // texture coordinates

uniform mat4 mvp;

out vec2 v_texcoord;
out vec3 v_colors;

void main()
{
	gl_Position = a_vertex * mvp;
	v_colors = a_colors;
	v_texcoord = a_texcoord;
}
