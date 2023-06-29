// Giant triangle that fills screen


attribute vec4 b_vertex; // vertex position 

uniform mat4 mvp;

void main()
{
	gl_Position = b_vertex * mvp;
}