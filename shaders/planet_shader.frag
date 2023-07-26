#version 300 es
precision mediump float;

in vec3 v_colors;
//in vec2 v_texcoord;
//in vec3 v_normal;
//in vec3 fragPos;

out vec4 FragColor;

void main()
{
    //vec3 norm = normalize(v_normal);

    FragColor = vec4(vec3(1.0, 1.0, 1.0), 1.0);
}
