#version 300 es

precision mediump float;

uniform vec4 pointColor;

out vec4 FragColor;

void main()
{
	float dist = distance(gl_PointCoord, vec2(0.5));
	if (dist > 0.5) {
		discard;
	}
	FragColor = pointColor;
}