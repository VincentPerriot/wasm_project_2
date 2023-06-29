attribute vec4 a_vertex; // vertex position 
attribute vec3 a_colors; // vertex colors
attribute vec2 a_texcoord; // texture coordinates

uniform mat4 mvp;

varying vec2 v_texcoord;
varying vec3 v_colors;

void main()
{
	gl_Position = a_vertex * mvp;
	v_colors = a_colors;
	v_texcoord = a_texcoord;
}
