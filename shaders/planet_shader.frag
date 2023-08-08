#version 300 es
precision mediump float;

in vec3 v_colors;
in vec3 v_normal;
in vec3 fragPos;
in vec3 initialPos;

uniform vec3 viewPos;
uniform int applyGradient;

out vec4 FragColor;

vec3 palette(float h)
{
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(0.618, -0.422, 0.808);
    vec3 d = vec3(-0.572, 0.388, 2.208);

    return a + b*cos( 6.28318*(c*h+d));
}
/*
vec3 palette(float h)
{
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263, 0.416, 0.557);

    return a + b*cos( 6.28318*(c*h+d));
}
*/

vec4 CreateLight(vec3 colors, vec3 lightPos, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir)
{

    //ambient
    float a_strength = 0.3;
    vec3 ambient = a_strength * lightColor;

    //diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    float s_strength = 0.8;
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 15.0);
    vec3 specular = s_strength * spec * lightColor;

    if (diffuse.x < 0.2  && diffuse.y < 0.2 && diffuse.z < 0.2)
    {
        specular = vec3(0.0);
    }

   return vec4(colors * (ambient + diffuse + specular), 1);
}

void main()
{
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 lightPos = vec3(0.0, 60.0, -20.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec3 colors = v_colors;

    if (applyGradient == 1)
    {
        float h = length(initialPos) * 5.0;
        colors = normalize(palette(h));
    }

    vec4 outColor = CreateLight(colors, lightPos, lightColor, v_normal, fragPos, viewDir);
    FragColor = outColor * vec4(colors, 1.0);
}