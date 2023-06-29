precision mediump float;

varying vec3 v_colors;
varying vec2 v_texcoord;

uniform sampler2D texture;

void main()
{
	gl_FragColor = texture2D(texture, v_texcoord) * vec4(v_colors, 1.0);
}
