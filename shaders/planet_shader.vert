#version 300 es

layout (location = 0) in vec4 a_vertex; // vertex position 
layout (location = 1) in vec3 a_colors; // vertex colors
layout (location = 2) in vec2 a_texcoord; // texture coordinates
layout (location = 3) in vec3 a_normal; // Normals Coordinates

uniform mat4 vp;
uniform mat4 model;

//out vec2 v_texcoord;
out vec3 v_colors;
//out vec3 v_normal;
//out vec3 fragPos;

void main()
{
	v_colors = a_colors;
	//v_texcoord = a_texcoord;
	//v_normal = mat3(transpose(inverse(model))) * a_normal;
	vec3 fragPos = vec3(a_vertex * model);
	gl_Position = vec4(fragPos, 1.0) * vp;
}
