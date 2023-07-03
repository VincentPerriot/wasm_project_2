#version 300 es
precision mediump float;

in vec3 v_colors;
in vec2 v_texcoord;

uniform sampler2D texture1;

out vec4 FragColor;

void main()
{
	FragColor = texture(texture1, v_texcoord) * vec4(v_colors, 0.6);
}
