#version 300 es

layout (location = 0) in vec4 a_vertex; // vertex position 
layout (location = 1) in vec3 a_colors; // vertex colors
layout (location = 3) in vec3 a_normal; // Normals Coordinates

uniform mat4 vp;
uniform mat4 model;

out vec3 v_normal;
out vec3 v_colors;
out vec3 fragPos;
out vec3 initialPos;

void main()
{
	v_colors = a_colors;

	v_normal = mat3(transpose(inverse(model))) * a_normal;

	initialPos = vec3(a_vertex);
	fragPos = vec3(a_vertex * model);
	gl_Position = vec4(fragPos, 1.0) * vp;
}
