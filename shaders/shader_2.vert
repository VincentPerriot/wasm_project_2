#version 300 es
layout (location = 0) in vec4 b_vertex; // vertex position 

uniform mat4 mvp;

void main()
{
	gl_Position = b_vertex * mvp;
}